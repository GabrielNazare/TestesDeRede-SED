#include "SimulationEngine.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <nlohmann/json.hpp>


SimulationEngine::SimulationEngine()
    : currentSimulationTime(0.0)
{}

void SimulationEngine::addBaseStation(const BaseStation& baseStation) {
    baseStations[baseStation.identifier] = baseStation;
}

void SimulationEngine::setMobileNode(const MobileNode& node) {
    mobileNode = std::make_unique<MobileNode>(node);
}

void SimulationEngine::scheduleEvent(const Event& simulationEvent) {
    eventQueue.push(simulationEvent);
}

void SimulationEngine::addObserver(std::shared_ptr<SimulationObserver> observer) {
    observers.push_back(std::move(observer));
}

void SimulationEngine::setFadingEnabled(bool enabled, double stdDev) {
    fadingEnabled = enabled;
    fadingShadowingStdDev = stdDev;
}

void SimulationEngine::setA3Parameters(double hysteresisDb, double tttSeconds) {
    a3HysteresisDb = hysteresisDb;
    timeToTriggerSeconds = tttSeconds;
}


void SimulationEngine::notifyMeasurement(double timestamp, const std::string& stationId, double rsrpDbm) {
    for (auto& observer : observers) {
        observer->onMeasurement(timestamp, stationId, rsrpDbm);
    }
}

void SimulationEngine::notifyHandover(double timestamp, const std::string& from, const std::string& to, double rsrp) {
    for (auto& observer : observers) {
        observer->onHandover(timestamp, from, to, rsrp);
    }
}

void SimulationEngine::notifyMovement(double timestamp, double x, double y) {
    for (auto& observer : observers) {
        observer->onMovement(timestamp, x, y);
    }
}


void SimulationEngine::loadSimulationDataFromJson(const std::string& jsonFilePath) {
    std::ifstream inputFile(jsonFilePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open file: " + jsonFilePath);
    }

    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(inputFile);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }

    double virtualTimestamp = 0.0;

    for (const auto& scanEntry : jsonData) {
        if (!scanEntry.contains("networks") || !scanEntry["networks"].is_array()) continue;

        for (const auto& network : scanEntry["networks"]) {
            std::string bssidValue = network.value("bssid", "");
            double rsrpValue       = network.value("rssi_dbm", -140.0);

            if (!bssidValue.empty() && rsrpValue > -140.0) {
                if (baseStations.find(bssidValue) == baseStations.end()) {
                    baseStations[bssidValue] = BaseStation(bssidValue);
                }

                Event measurementEvent{};
                measurementEvent.timestampSeconds    = virtualTimestamp;
                measurementEvent.type                = EventType::SIGNAL_MEASUREMENT;
                measurementEvent.sourceBaseStationId = bssidValue;
                measurementEvent.rsrpValueDbm        = rsrpValue;
                scheduleEvent(measurementEvent);
            }
        }
        virtualTimestamp += 3.0;
    }

    Event endEvent{};
    endEvent.timestampSeconds = virtualTimestamp;
    endEvent.type = EventType::SIMULATION_END;
    scheduleEvent(endEvent);

    std::cout << "[DES] Loaded " << eventQueue.size() << " events from '" << jsonFilePath << "'.\n";
}


void SimulationEngine::startSimulation() {
    if (!mobileNode) throw std::runtime_error("MobileNode not configured.");
    if (baseStations.empty()) throw std::runtime_error("No BaseStations configured.");

    mobileNode->connectedBaseStationId = baseStations.begin()->first;
    mobileNode->currentRsrpDbm         = baseStations.begin()->second.currentRsrpValueDbm;

    for (auto& observer : observers) {
        observer->onSimulationStart(mobileNode->identifier, mobileNode->connectedBaseStationId);
    }

    std::cout << "\n[DES] ===== STARTING SIMULATION (3GPP A3 Event + TTT="
              << timeToTriggerSeconds * 1000 << "ms, Hys=" << a3HysteresisDb << "dB) =====\n";
    std::cout << "[DES] UE: " << mobileNode->identifier
              << " | Connected to: " << mobileNode->connectedBaseStationId << "\n\n";

    while (!eventQueue.empty()) {
        Event currentEvent = eventQueue.top();
        eventQueue.pop();
        currentSimulationTime = currentEvent.timestampSeconds;

        switch (currentEvent.type) {
            case EventType::SIGNAL_MEASUREMENT: processSignalMeasurement(currentEvent); break;
            case EventType::HANDOVER_TRIGGER:   processHandoverTrigger(currentEvent);   break;
            case EventType::USER_MOVEMENT:      processUserMovement(currentEvent);      break;
            case EventType::SIMULATION_END:
                std::cout << "[DES] t=" << std::fixed << std::setprecision(1)
                          << currentSimulationTime << "s | SIMULATION_END\n";
                for (auto& observer : observers) {
                    observer->onSimulationEnd(currentSimulationTime);
                }
                return;
        }
    }
}


void SimulationEngine::processSignalMeasurement(const Event& measurementEvent) {
    if (measurementEvent.rsrpValueDbm > -140.0) {
        if (baseStations.count(measurementEvent.sourceBaseStationId)) {
            baseStations[measurementEvent.sourceBaseStationId].currentRsrpValueDbm = measurementEvent.rsrpValueDbm;
        }
    }

    if (measurementEvent.sourceBaseStationId == mobileNode->connectedBaseStationId) {
        mobileNode->currentRsrpDbm = baseStations[measurementEvent.sourceBaseStationId].currentRsrpValueDbm;
    }

    notifyMeasurement(measurementEvent.timestampSeconds,
                      measurementEvent.sourceBaseStationId,
                      measurementEvent.rsrpValueDbm);

    std::string bestTargetId = evaluateA3EventWithTTT(measurementEvent.timestampSeconds);
    if (!bestTargetId.empty() && bestTargetId != mobileNode->connectedBaseStationId) {
        Event triggerEvent{};
        triggerEvent.timestampSeconds    = measurementEvent.timestampSeconds + 0.01;
        triggerEvent.type                = EventType::HANDOVER_TRIGGER;
        triggerEvent.sourceBaseStationId = mobileNode->connectedBaseStationId;
        triggerEvent.targetBaseStationId = bestTargetId;
        triggerEvent.rsrpValueDbm        = baseStations[bestTargetId].currentRsrpValueDbm;
        scheduleEvent(triggerEvent);
    }
}

void SimulationEngine::processHandoverTrigger(const Event& triggerEvent) {
    if (!baseStations.count(triggerEvent.targetBaseStationId)) return;
    if (triggerEvent.targetBaseStationId == mobileNode->connectedBaseStationId) return;

    const BaseStation& targetStation = baseStations[triggerEvent.targetBaseStationId];
    std::string previousStationId    = mobileNode->connectedBaseStationId;

    mobileNode->performHandoverTo(targetStation);
    pendingA3EventTimestamps.erase(triggerEvent.targetBaseStationId);

    std::string logEntry =
        "t=" + std::to_string((int)triggerEvent.timestampSeconds) + "s | "
        "A3 HANDOVER: " + previousStationId + " -> " + targetStation.identifier +
        " | RSRP: " + std::to_string((int)triggerEvent.rsrpValueDbm) + " dBm";

    handoverHistoryLog.push_back(logEntry);

    notifyHandover(triggerEvent.timestampSeconds, previousStationId,
                   targetStation.identifier, triggerEvent.rsrpValueDbm);

    std::cout << "[DES] t=" << std::fixed << std::setprecision(1)
              << triggerEvent.timestampSeconds << "s | *** A3 HANDOVER *** | "
              << previousStationId << " -> " << targetStation.identifier
              << " (RSRP: " << std::setprecision(1) << triggerEvent.rsrpValueDbm << " dBm)\n";
}

void SimulationEngine::processUserMovement(const Event& movementEvent) {
    mobileNode->positionX = movementEvent.newPositionX;
    mobileNode->positionY = movementEvent.newPositionY;

    for (auto& [stationId, baseStation] : baseStations) {
        baseStation.currentRsrpValueDbm =
            calculateLogDistancePathLoss(baseStation, mobileNode->positionX, mobileNode->positionY);
    }

    notifyMovement(movementEvent.timestampSeconds, mobileNode->positionX, mobileNode->positionY);

    std::cout << "[DES] t=" << std::fixed << std::setprecision(1)
              << movementEvent.timestampSeconds << "s | MOVEMENT | UE at ("
              << mobileNode->positionX << ", " << mobileNode->positionY << ")\n";
}


std::string SimulationEngine::evaluateA3EventWithTTT(double currentTimestamp) {
    if (!mobileNode) return "";

    double servingRsrp = mobileNode->currentRsrpDbm;
    std::string bestCandidateId = "";
    double bestCandidateRsrp = servingRsrp;

    for (const auto& [stationId, baseStation] : baseStations) {
        if (stationId == mobileNode->connectedBaseStationId) continue;

        bool a3ConditionIsMet = handover::isA3ConditionMet(
            baseStation.currentRsrpValueDbm, servingRsrp, a3HysteresisDb);

        if (a3ConditionIsMet) {
            if (pendingA3EventTimestamps.find(stationId) == pendingA3EventTimestamps.end()) {
                pendingA3EventTimestamps[stationId] = currentTimestamp;
            }

            double a3FirstDetectedAt = pendingA3EventTimestamps[stationId];
            bool tttHasExpired = (currentTimestamp - a3FirstDetectedAt) >= timeToTriggerSeconds;

            if (tttHasExpired && baseStation.currentRsrpValueDbm > bestCandidateRsrp) {
                bestCandidateRsrp = baseStation.currentRsrpValueDbm;
                bestCandidateId   = stationId;
            }
        } else {
            pendingA3EventTimestamps.erase(stationId);
        }
    }

    return bestCandidateId;
}


double SimulationEngine::calculateLogDistancePathLoss(
    const BaseStation& station, double x, double y)
{
    double distance = handover::euclideanDistance(station.positionX, station.positionY, x, y);
    double baseRsrp = handover::calculateLogDistancePathLoss(
        station.transmissionPowerDbm, distance);

    if (fadingEnabled) {
        return handover::applyRayleighFading(baseRsrp, rng, fadingShadowingStdDev);
    }

    return baseRsrp;
}


void SimulationEngine::printSimulationSummary() const {
    std::cout << "\n[DES] ===== SIMULATION SUMMARY =====\n";
    std::cout << "[DES] Total A3 Handovers : " << handoverHistoryLog.size() << "\n";
    std::cout << "[DES] Stations detected  : " << baseStations.size() << "\n\n";

    if (handoverHistoryLog.empty()) {
        std::cout << "[DES] No handovers occurred.\n";
    } else {
        std::cout << "[DES] Handover History:\n";
        for (const auto& logEntry : handoverHistoryLog) {
            std::cout << "       > " << logEntry << "\n";
        }
    }
    std::cout << "[DES] ====================================\n\n";
}
