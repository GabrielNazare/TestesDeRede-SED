#pragma once
#include <string>

class SimulationObserver {
public:
    virtual ~SimulationObserver() = default;

    virtual void onSimulationStart(
        const std::string& ueId,
        const std::string& initialStationId) {}

    virtual void onMeasurement(
        double timestampSeconds,
        const std::string& stationId,
        double rsrpDbm) {}

    virtual void onHandover(
        double timestampSeconds,
        const std::string& fromStationId,
        const std::string& toStationId,
        double targetRsrpDbm) {}

    virtual void onMovement(
        double timestampSeconds,
        double positionX,
        double positionY) {}

    virtual void onSimulationEnd(double timestampSeconds) {}
};
