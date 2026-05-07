#include <gtest/gtest.h>
#include "SimulationEngine.h"
#include "MetricsCollector.h"
#include <memory>


class PingPongTest : public ::testing::Test {
protected:
    SimulationEngine engine;
    std::shared_ptr<MetricsCollector> metrics;

    void SetUp() override {
        metrics = std::make_shared<MetricsCollector>();
        engine.addObserver(metrics);
    }

    void scheduleMeasurement(const std::string& id, double rsrp, double t) {
        Event e{};
        e.timestampSeconds = t;
        e.type = EventType::SIGNAL_MEASUREMENT;
        e.sourceBaseStationId = id;
        e.rsrpValueDbm = rsrp;
        engine.scheduleEvent(e);
    }
};

TEST_F(PingPongTest, HysteresisPreventsPingPongWithSimilarSignals) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -69.0));
    engine.addBaseStation(BaseStation("eNB-B", 50, 0, 20.0, -71.0));
    engine.setMobileNode(MobileNode("UE-001"));

    double t = 0.0;
    for (int i = 0; i < 30; ++i) {
        double rsrpA = (i % 2 == 0) ? -68.0 : -72.0;
        double rsrpB = (i % 2 == 0) ? -72.0 : -68.0;

        scheduleMeasurement("eNB-A", rsrpA, t);
        scheduleMeasurement("eNB-B", rsrpB, t + 0.001);
        t += 0.5;
    }

    Event end{};
    end.timestampSeconds = t;
    end.type = EventType::SIMULATION_END;
    engine.scheduleEvent(end);

    engine.startSimulation();

    EXPECT_LE(engine.getHandoverCount(), 2u)
        << "Hysteresis should prevent excessive ping-pong";
}

TEST_F(PingPongTest, PingPongRateIsLow) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -69.0));
    engine.addBaseStation(BaseStation("eNB-B", 50, 0, 20.0, -71.0));
    engine.setMobileNode(MobileNode("UE-001"));

    double t = 0.0;
    for (int i = 0; i < 30; ++i) {
        double rsrpA = (i % 2 == 0) ? -68.0 : -72.0;
        double rsrpB = (i % 2 == 0) ? -72.0 : -68.0;
        scheduleMeasurement("eNB-A", rsrpA, t);
        scheduleMeasurement("eNB-B", rsrpB, t + 0.001);
        t += 0.5;
    }

    Event end{};
    end.timestampSeconds = t;
    end.type = EventType::SIMULATION_END;
    engine.scheduleEvent(end);

    engine.startSimulation();

    double pingPongRate = metrics->getPingPongRate(5.0);
    EXPECT_LE(pingPongRate, 50.0)
        << "Ping-pong rate too high — hysteresis not effective";
}

TEST_F(PingPongTest, ClearHandoverNoPingPong) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -60.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -80.0));
    engine.setMobileNode(MobileNode("UE-001"));

    double t = 0.0;
    for (int i = 0; i < 20; ++i) {
        double rsrpA = -60.0 - (i * 2.0);   // weakening
        double rsrpB = -80.0 + (i * 2.0);   // strengthening
        scheduleMeasurement("eNB-A", rsrpA, t);
        scheduleMeasurement("eNB-B", rsrpB, t + 0.001);
        t += 0.2;
    }

    Event end{};
    end.timestampSeconds = t;
    end.type = EventType::SIMULATION_END;
    engine.scheduleEvent(end);

    engine.startSimulation();

    EXPECT_NEAR(metrics->getPingPongRate(), 0.0, 0.1)
        << "Clear signal transition should produce 0% ping-pong";
}
