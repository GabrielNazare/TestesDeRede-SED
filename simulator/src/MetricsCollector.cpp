#include "MetricsCollector.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <filesystem>
#include <cmath>


void MetricsCollector::onSimulationStart(
    const std::string& ueId,
    const std::string& initialStationId)
{
    ueIdentifier = ueId;
}

void MetricsCollector::onMeasurement(
    double timestampSeconds,
    const std::string& stationId,
    double rsrpDbm)
{
    measurementRecords.push_back({timestampSeconds, stationId, rsrpDbm});
}

void MetricsCollector::onHandover(
    double timestampSeconds,
    const std::string& fromStationId,
    const std::string& toStationId,
    double targetRsrpDbm)
{
    handoverRecords.push_back({timestampSeconds, fromStationId, toStationId, targetRsrpDbm});
}

void MetricsCollector::onMovement(
    double timestampSeconds,
    double positionX,
    double positionY)
{
    positionRecords.push_back({timestampSeconds, positionX, positionY});
}

void MetricsCollector::onSimulationEnd(double timestampSeconds) {
    simulationEndTime = timestampSeconds;
}


size_t MetricsCollector::getTotalHandovers() const {
    return handoverRecords.size();
}

double MetricsCollector::getPingPongRate(double windowSeconds) const {
    if (handoverRecords.size() < 2) return 0.0;

    int pingPongCount = 0;
    for (size_t i = 1; i < handoverRecords.size(); ++i) {
        bool returnsToSource =
            handoverRecords[i].toStation == handoverRecords[i - 1].fromStation;
        bool withinWindow =
            (handoverRecords[i].timestamp - handoverRecords[i - 1].timestamp) < windowSeconds;

        if (returnsToSource && withinWindow) {
            ++pingPongCount;
        }
    }

    return (static_cast<double>(pingPongCount) / static_cast<double>(handoverRecords.size())) * 100.0;
}

double MetricsCollector::getAverageRsrp() const {
    if (measurementRecords.empty()) return -140.0;

    double sum = std::accumulate(
        measurementRecords.begin(), measurementRecords.end(), 0.0,
        [](double acc, const MeasurementRecord& record) {
            return acc + record.rsrpDbm;
        });

    return sum / static_cast<double>(measurementRecords.size());
}

double MetricsCollector::getCoveragePercent(double thresholdDbm) const {
    if (measurementRecords.empty()) return 0.0;

    long aboveThreshold = std::count_if(
        measurementRecords.begin(), measurementRecords.end(),
        [thresholdDbm](const MeasurementRecord& record) {
            return record.rsrpDbm > thresholdDbm;
        });

    return (static_cast<double>(aboveThreshold) / static_cast<double>(measurementRecords.size())) * 100.0;
}


void MetricsCollector::exportToCsv(const std::string& outputDirectory) const {
    std::filesystem::create_directories(outputDirectory);

    {
        std::ofstream file(outputDirectory + "/handover_events.csv");
        file << "timestamp_s,from_station,to_station,rsrp_dbm\n";
        for (const auto& record : handoverRecords) {
            file << std::fixed << std::setprecision(3)
                 << record.timestamp << ","
                 << record.fromStation << ","
                 << record.toStation << ","
                 << record.rsrpDbm << "\n";
        }
    }

    {
        std::ofstream file(outputDirectory + "/signal_trace.csv");
        file << "timestamp_s,station_id,rsrp_dbm\n";
        for (const auto& record : measurementRecords) {
            file << std::fixed << std::setprecision(3)
                 << record.timestamp << ","
                 << record.stationId << ","
                 << record.rsrpDbm << "\n";
        }
    }

    {
        std::ofstream file(outputDirectory + "/position_trace.csv");
        file << "timestamp_s,x,y\n";
        for (const auto& record : positionRecords) {
            file << std::fixed << std::setprecision(3)
                 << record.timestamp << ","
                 << record.x << ","
                 << record.y << "\n";
        }
    }

    {
        std::ofstream file(outputDirectory + "/simulation_metrics.csv");
        file << "metric,value\n";
        file << "total_handovers," << getTotalHandovers() << "\n";
        file << "ping_pong_rate_pct," << std::fixed << std::setprecision(2) << getPingPongRate() << "\n";
        file << "average_rsrp_dbm," << std::fixed << std::setprecision(2) << getAverageRsrp() << "\n";
        file << "coverage_pct," << std::fixed << std::setprecision(2) << getCoveragePercent() << "\n";
        file << "total_measurements," << measurementRecords.size() << "\n";
        file << "simulation_duration_s," << std::fixed << std::setprecision(2) << simulationEndTime << "\n";
    }

    std::cout << "[METRICS] CSV files exported to: " << outputDirectory << "/\n";
}


void MetricsCollector::printKpiSummary() const {
    std::cout << "\n[KPI] ===== TELECOM KPIs =====\n";
    std::cout << "[KPI] Total Handovers     : " << getTotalHandovers() << "\n";
    std::cout << "[KPI] Ping-Pong Rate      : " << std::fixed << std::setprecision(1)
              << getPingPongRate() << " %\n";
    std::cout << "[KPI] Average RSRP        : " << std::fixed << std::setprecision(1)
              << getAverageRsrp() << " dBm\n";
    std::cout << "[KPI] Coverage (>-110 dBm): " << std::fixed << std::setprecision(1)
              << getCoveragePercent() << " %\n";
    std::cout << "[KPI] Total Measurements  : " << measurementRecords.size() << "\n";
    std::cout << "[KPI] ============================\n\n";
}
