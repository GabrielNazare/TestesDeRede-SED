#pragma once
#include <queue>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <random>
#include "Event.h"
#include "MobileNode.h"
#include "BaseStation.h"
#include "SignalFilter.h"
#include "SimulationObserver.h"

namespace handover {

inline bool isA3ConditionMet(double candidateRsrpDbm, double servingRsrpDbm, double hysteresisDb) {
    return candidateRsrpDbm > (servingRsrpDbm + hysteresisDb);
}

inline double calculateLogDistancePathLoss(
    double txPowerDbm, double distanceMeters, double pathLossExponent = 2.7)
{
    static constexpr double MINIMUM_DISTANCE_METERS = 1.0;
    distanceMeters = std::max(distanceMeters, MINIMUM_DISTANCE_METERS);
    return txPowerDbm - 10.0 * pathLossExponent * std::log10(distanceMeters);
}

inline double applyRayleighFading(double baseRsrpDbm, std::mt19937& rng, double shadowingStdDev = 4.0) {
    std::normal_distribution<double> fadingDistribution(0.0, shadowingStdDev);
    return baseRsrpDbm + fadingDistribution(rng);
}

inline double euclideanDistance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return std::sqrt(dx * dx + dy * dy);
}

} 

class SimulationEngine {
public:
    SimulationEngine();

    void addBaseStation(const BaseStation& baseStation);
    void setMobileNode(const MobileNode& mobileNode);
    void scheduleEvent(const Event& simulationEvent);
    void loadSimulationDataFromJson(const std::string& jsonFilePath);
    void startSimulation();
    void printSimulationSummary() const;

    void addObserver(std::shared_ptr<SimulationObserver> observer);

    void setA3Parameters(double hysteresisDb, double tttSeconds);
    void setFadingEnabled(bool enabled, double stdDev = 4.0);

    double getHysteresisDb() const { return a3HysteresisDb; }
    double getTttSeconds() const { return timeToTriggerSeconds; }

    size_t getHandoverCount() const { return handoverHistoryLog.size(); }
    size_t getBaseStationCount() const { return baseStations.size(); }
    const MobileNode* getMobileNode() const { return mobileNode.get(); }
    const std::vector<std::string>& getHandoverLog() const { return handoverHistoryLog; }

private:
    using EventPriorityQueue = std::priority_queue<Event, std::vector<Event>, std::greater<Event>>;

    EventPriorityQueue                            eventQueue;
    std::unique_ptr<MobileNode>                   mobileNode;
    std::unordered_map<std::string, BaseStation>  baseStations;
    std::vector<std::string>                      handoverHistoryLog;
    double                                        currentSimulationTime;

    std::unordered_map<std::string, double>       pendingA3EventTimestamps;

    std::vector<std::shared_ptr<SimulationObserver>> observers;

    bool   fadingEnabled          = false;
    double fadingShadowingStdDev   = 4.0;
    double a3HysteresisDb          = 3.0;
    double timeToTriggerSeconds    = 0.16;
    std::mt19937 rng{42};

    void processSignalMeasurement(const Event& measurementEvent);
    void processHandoverTrigger(const Event& triggerEvent);
    void processUserMovement(const Event& movementEvent);
    std::string evaluateA3EventWithTTT(double currentTimestamp);
    double calculateLogDistancePathLoss(const BaseStation& station, double x, double y);

    void notifyMeasurement(double timestamp, const std::string& stationId, double rsrpDbm);
    void notifyHandover(double timestamp, const std::string& from, const std::string& to, double rsrp);
    void notifyMovement(double timestamp, double x, double y);
};
