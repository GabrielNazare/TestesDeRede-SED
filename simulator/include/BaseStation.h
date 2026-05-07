#pragma once
#include <string>

struct BaseStation {
    std::string identifier;
    double      positionX;
    double      positionY;
    double      transmissionPowerDbm;
    double      currentRsrpValueDbm;

    BaseStation()
        : identifier(""), positionX(0.0), positionY(0.0),
          transmissionPowerDbm(20.0), currentRsrpValueDbm(-100.0) {}

    BaseStation(
        const std::string& id,
        double x = 0.0,
        double y = 0.0,
        double power = 20.0,
        double initialRsrp = -100.0
    )
        : identifier(id), positionX(x), positionY(y),
          transmissionPowerDbm(power), currentRsrpValueDbm(initialRsrp)
    {}
};
