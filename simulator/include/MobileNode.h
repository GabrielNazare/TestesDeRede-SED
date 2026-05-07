#pragma once
#include <string>
#include "BaseStation.h"

struct MobileNode {
    std::string  identifier;
    double       positionX;
    double       positionY;
    double       currentRsrpDbm;
    std::string  connectedBaseStationId;
    int          totalHandoverCount;

    MobileNode(
        const std::string& id,
        double x = 0.0,
        double y = 0.0
    )
        : identifier(id), positionX(x), positionY(y),
          currentRsrpDbm(-100.0),
          connectedBaseStationId(""),
          totalHandoverCount(0)
    {}

    void performHandoverTo(const BaseStation& targetBaseStation) {
        connectedBaseStationId = targetBaseStation.identifier;
        currentRsrpDbm         = targetBaseStation.currentRsrpValueDbm;
        ++totalHandoverCount;
    }
};
