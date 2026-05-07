#include <gtest/gtest.h>
#include "SimulationEngine.h"
#include <cmath>
#include <random>


TEST(PathLossTest, AtMinimumDistanceEqualsTransmitPower) {
    double result = handover::calculateLogDistancePathLoss(20.0, 1.0, 2.7);
    EXPECT_NEAR(result, 20.0, 0.001);
}

TEST(PathLossTest, At10Meters) {
    double result = handover::calculateLogDistancePathLoss(20.0, 10.0, 2.7);
    EXPECT_NEAR(result, -7.0, 0.001);
}

TEST(PathLossTest, At100Meters) {
    double result = handover::calculateLogDistancePathLoss(20.0, 100.0, 2.7);
    EXPECT_NEAR(result, -34.0, 0.001);
}

TEST(PathLossTest, ZeroDistanceClamped) {
    double result = handover::calculateLogDistancePathLoss(20.0, 0.0, 2.7);
    EXPECT_NEAR(result, 20.0, 0.001);
}

TEST(PathLossTest, NegativeDistanceClamped) {
    double result = handover::calculateLogDistancePathLoss(20.0, -5.0, 2.7);
    EXPECT_NEAR(result, 20.0, 0.001);
}

TEST(PathLossTest, FreeSpaceExponent) {
    double result = handover::calculateLogDistancePathLoss(20.0, 100.0, 2.0);
    EXPECT_NEAR(result, -20.0, 0.001);
}

TEST(PathLossTest, DenseUrbanExponent) {
    double result = handover::calculateLogDistancePathLoss(20.0, 100.0, 4.0);
    EXPECT_NEAR(result, -60.0, 0.001);
}

TEST(PathLossTest, MonotonicallyDecreasing) {
    double prev = handover::calculateLogDistancePathLoss(20.0, 1.0, 2.7);
    for (double d = 10.0; d <= 500.0; d += 50.0) {
        double current = handover::calculateLogDistancePathLoss(20.0, d, 2.7);
        EXPECT_LT(current, prev);
        prev = current;
    }
}


TEST(EuclideanDistanceTest, SamePoint) {
    EXPECT_NEAR(handover::euclideanDistance(5, 5, 5, 5), 0.0, 0.001);
}

TEST(EuclideanDistanceTest, Horizontal) {
    EXPECT_NEAR(handover::euclideanDistance(0, 0, 100, 0), 100.0, 0.001);
}

TEST(EuclideanDistanceTest, Triangle345) {
    EXPECT_NEAR(handover::euclideanDistance(0, 0, 3, 4), 5.0, 0.001);
}


TEST(FadingTest, AddsVariability) {
    std::mt19937 rng(42);
    bool hasDiff = false;
    for (int i = 0; i < 100; ++i) {
        if (std::fabs(handover::applyRayleighFading(-70.0, rng, 4.0) - (-70.0)) > 0.01) {
            hasDiff = true;
            break;
        }
    }
    EXPECT_TRUE(hasDiff);
}

TEST(FadingTest, MeanApproximatesBase) {
    std::mt19937 rng(42);
    double sum = 0.0;
    int N = 10000;
    for (int i = 0; i < N; ++i) sum += handover::applyRayleighFading(-70.0, rng, 4.0);
    EXPECT_NEAR(sum / N, -70.0, 0.5);
}
