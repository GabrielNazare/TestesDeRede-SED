#pragma once
#include "SimulationObserver.h"
#include <vector>
#include <string>
#include <tuple>


struct HandoverRecord {
    double timestamp;
    std::string fromStation;
    std::string toStation;
    double rsrpDbm;
};

struct MeasurementRecord {
    double timestamp;
    std::string stationId;
    double rsrpDbm;
};

struct PositionRecord {
    double timestamp;
    double x;
    double y;
};

class MetricsCollector : public SimulationObserver {
public:
    void onSimulationStart(
        const std::string& ueId,
        const std::string& initialStationId) override;

    void onMeasurement(
        double timestampSeconds,
        const std::string& stationId,
        double rsrpDbm) override;

    void onHandover(
        double timestampSeconds,
        const std::string& fromStationId,
        const std::string& toStationId,
        double targetRsrpDbm) override;

    void onMovement(
        double timestampSeconds,
        double positionX,
        double positionY) override;

    void onSimulationEnd(double timestampSeconds) override;


    size_t getTotalHandovers() const;

    double getPingPongRate(double windowSeconds = 5.0) const;

    double getAverageRsrp() const;

    double getCoveragePercent(double thresholdDbm = -110.0) const;

    void exportToCsv(const std::string& outputDirectory) const;

    void printKpiSummary() const;

    const std::vector<HandoverRecord>& getHandovers() const { return handoverRecords; }
    const std::vector<MeasurementRecord>& getMeasurements() const { return measurementRecords; }

private:
    std::string ueIdentifier;
    double simulationStartTime = 0.0;
    double simulationEndTime   = 0.0;

    std::vector<HandoverRecord>     handoverRecords;
    std::vector<MeasurementRecord>  measurementRecords;
    std::vector<PositionRecord>     positionRecords;
};
