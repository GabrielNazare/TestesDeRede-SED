#include <gtest/gtest.h>
#include "SimulationEngine.h"
#include "MetricsCollector.h"
#include <memory>


class TTTTest : public ::testing::Test {
protected:
    SimulationEngine engine;
    std::shared_ptr<MetricsCollector> metrics;

    void SetUp() override {
        metrics = std::make_shared<MetricsCollector>();
        engine.addObserver(metrics);
    }

    void scheduleMeasurement(const std::string& stationId, double rsrpDbm, double timestamp) {
        Event e{};
        e.timestampSeconds    = timestamp;
        e.type                = EventType::SIGNAL_MEASUREMENT;
        e.sourceBaseStationId = stationId;
        e.rsrpValueDbm        = rsrpDbm;
        engine.scheduleEvent(e);
    }

    void scheduleEnd(double timestamp) {
        Event e{};
        e.timestampSeconds = timestamp;
        e.type = EventType::SIMULATION_END;
        engine.scheduleEvent(e);
    }
};

TEST_F(TTTTest, NoHandoverBeforeTTTExpires) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -70.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -60.0));
    engine.setMobileNode(MobileNode("UE-001"));

    scheduleMeasurement("eNB-A", -70.0, 0.0);
    scheduleMeasurement("eNB-B", -60.0, 0.01);
    scheduleMeasurement("eNB-A", -70.0, 0.05);
    scheduleMeasurement("eNB-B", -60.0, 0.06);
    scheduleEnd(0.1);

    engine.startSimulation();
    EXPECT_EQ(engine.getHandoverCount(), 0u);
}

TEST_F(TTTTest, HandoverAfterTTTExpires) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -70.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -60.0));
    engine.setMobileNode(MobileNode("UE-001"));

    for (double t = 0.0; t <= 0.5; t += 0.05) {
        scheduleMeasurement("eNB-A", -70.0, t);
        scheduleMeasurement("eNB-B", -60.0, t + 0.001);
    }
    scheduleEnd(1.0);

    engine.startSimulation();
    EXPECT_GE(engine.getHandoverCount(), 1u);
}

TEST_F(TTTTest, A3ConditionLostBeforeTTTResetsTimer) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -70.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -60.0));
    engine.setMobileNode(MobileNode("UE-001"));

    scheduleMeasurement("eNB-A", -70.0, 0.0);
    scheduleMeasurement("eNB-B", -60.0, 0.01);

    scheduleMeasurement("eNB-A", -70.0, 0.08);
    scheduleMeasurement("eNB-B", -72.0, 0.081);

    scheduleMeasurement("eNB-A", -70.0, 0.10);
    scheduleMeasurement("eNB-B", -60.0, 0.101);

    scheduleEnd(0.20);

    engine.startSimulation();
    EXPECT_EQ(engine.getHandoverCount(), 0u);
}

TEST_F(TTTTest, MultipleCandidatesSelectsBestAfterTTT) {
    engine.addBaseStation(BaseStation("eNB-A", 0, 0, 20.0, -70.0));
    engine.addBaseStation(BaseStation("eNB-B", 100, 0, 20.0, -60.0));
    engine.addBaseStation(BaseStation("eNB-C", 200, 0, 20.0, -55.0));
    engine.setMobileNode(MobileNode("UE-001"));

    for (double t = 0.0; t <= 0.5; t += 0.05) {
        scheduleMeasurement("eNB-A", -70.0, t);
        scheduleMeasurement("eNB-B", -60.0, t + 0.001);
        scheduleMeasurement("eNB-C", -55.0, t + 0.002);
    }
    scheduleEnd(1.0);

    engine.startSimulation();

    EXPECT_GE(engine.getHandoverCount(), 1u);
    const auto* ue = engine.getMobileNode();
    ASSERT_NE(ue, nullptr);
    EXPECT_EQ(ue->connectedBaseStationId, "eNB-C");
}
