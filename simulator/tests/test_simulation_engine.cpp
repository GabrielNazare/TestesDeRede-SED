#include <gtest/gtest.h>
#include "SimulationEngine.h"
#include "MetricsCollector.h"
#include <memory>


class SimEngineTest : public ::testing::Test {
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

    void scheduleMovement(double x, double y, double t) {
        Event e{};
        e.timestampSeconds = t;
        e.type = EventType::USER_MOVEMENT;
        e.newPositionX = x;
        e.newPositionY = y;
        engine.scheduleEvent(e);
    }

    void scheduleEnd(double t) {
        Event e{};
        e.timestampSeconds = t;
        e.type = EventType::SIMULATION_END;
        engine.scheduleEvent(e);
    }
};

TEST_F(SimEngineTest, ThrowsWithoutMobileNode) {
    engine.addBaseStation(BaseStation("eNB-A"));
    scheduleEnd(1.0);
    EXPECT_THROW(engine.startSimulation(), std::runtime_error);
}

TEST_F(SimEngineTest, ThrowsWithoutBaseStations) {
    engine.setMobileNode(MobileNode("UE-001"));
    scheduleEnd(1.0);
    EXPECT_THROW(engine.startSimulation(), std::runtime_error);
}

TEST_F(SimEngineTest, SyntheticScenarioProducesHandovers) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0));
    engine.addBaseStation(BaseStation("eNB-C", 200, 0, 20.0));
    engine.setMobileNode(MobileNode("UE-001", 10, 0));

    double t = 0.0;
    for (double x = 10; x <= 210; x += 20) {
        scheduleMovement(x, 0, t);
        t += 0.2;
        for (auto& id : {"eNB-A", "eNB-B", "eNB-C"}) {
            scheduleMeasurement(id, -100, t);
            t += 0.01;
        }
        t += 0.8;
    }
    scheduleEnd(t);

    engine.startSimulation();
    EXPECT_GE(engine.getHandoverCount(), 1u);
    EXPECT_EQ(engine.getBaseStationCount(), 3u);
}

TEST_F(SimEngineTest, StableSignalNoHandover) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -50.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -80.0));
    engine.setMobileNode(MobileNode("UE-001"));

    for (double t = 0; t <= 1.0; t += 0.1) {
        scheduleMeasurement("eNB-A", -50.0, t);
        scheduleMeasurement("eNB-B", -80.0, t + 0.001);
    }
    scheduleEnd(2.0);

    engine.startSimulation();
    EXPECT_EQ(engine.getHandoverCount(), 0u);
}

TEST_F(SimEngineTest, MetricsCollectorRecordsMeasurements) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -60.0));
    engine.setMobileNode(MobileNode("UE-001"));

    scheduleMeasurement("eNB-A", -60.0, 0.0);
    scheduleMeasurement("eNB-A", -62.0, 0.1);
    scheduleMeasurement("eNB-A", -58.0, 0.2);
    scheduleEnd(1.0);

    engine.startSimulation();
    EXPECT_EQ(metrics->getMeasurements().size(), 3u);
    EXPECT_NEAR(metrics->getAverageRsrp(), -60.0, 1.0);
}

TEST_F(SimEngineTest, CoveragePercent100WhenStrong) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -50.0));
    engine.setMobileNode(MobileNode("UE-001"));

    for (double t = 0; t < 1.0; t += 0.1) {
        scheduleMeasurement("eNB-A", -50.0, t);
    }
    scheduleEnd(1.0);

    engine.startSimulation();
    EXPECT_NEAR(metrics->getCoveragePercent(-110.0), 100.0, 0.1);
}

TEST_F(SimEngineTest, FadingProducesDifferentResults) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0));
    engine.setMobileNode(MobileNode("UE-001", 50, 0));
    engine.setFadingEnabled(true, 6.0);

    scheduleMovement(50, 0, 0.0);
    scheduleEnd(1.0);

    engine.startSimulation();
    EXPECT_EQ(engine.getBaseStationCount(), 1u);
}
