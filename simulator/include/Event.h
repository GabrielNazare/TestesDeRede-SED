#pragma once
#include <string>

enum class EventType {
    SIGNAL_MEASUREMENT,
    HANDOVER_TRIGGER,
    USER_MOVEMENT,
    SIMULATION_END
};

inline std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::SIGNAL_MEASUREMENT: return "SIGNAL_MEASUREMENT";
        case EventType::HANDOVER_TRIGGER:   return "HANDOVER_TRIGGER";
        case EventType::USER_MOVEMENT:      return "USER_MOVEMENT";
        case EventType::SIMULATION_END:     return "SIMULATION_END";
        default:                            return "UNKNOWN";
    }
}

struct Event {
    double      timestampSeconds;
    EventType   type;
    std::string sourceBaseStationId;
    std::string targetBaseStationId;
    double      rsrpValueDbm;
    double      newPositionX;
    double      newPositionY;

    bool operator>(const Event& other) const {
        return timestampSeconds > other.timestampSeconds;
    }
};
