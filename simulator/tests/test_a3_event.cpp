#include <gtest/gtest.h>
#include "SimulationEngine.h"


TEST(A3EventTest, HandoverTriggersWhenCandidateStrongerThanHysteresis) {
    EXPECT_TRUE(handover::isA3ConditionMet(-65.0, -71.0, 3.0));
}

TEST(A3EventTest, NoHandoverWhenExactlyAtHysteresisThreshold) {
    EXPECT_FALSE(handover::isA3ConditionMet(-68.0, -71.0, 3.0));
}

TEST(A3EventTest, NoHandoverWhenCandidateWeaker) {
    EXPECT_FALSE(handover::isA3ConditionMet(-75.0, -70.0, 3.0));
}

TEST(A3EventTest, NoHandoverWhenCandidateSlightlyStronger) {
    EXPECT_FALSE(handover::isA3ConditionMet(-69.0, -70.0, 3.0));
}

TEST(A3EventTest, HandoverWithZeroHysteresis) {
    EXPECT_TRUE(handover::isA3ConditionMet(-69.0, -70.0, 0.0));
}

TEST(A3EventTest, HandoverWithLargeHysteresis) {
    EXPECT_FALSE(handover::isA3ConditionMet(-60.0, -70.0, 10.0));
    EXPECT_TRUE(handover::isA3ConditionMet(-59.0, -70.0, 10.0));
}

TEST(A3EventTest, BothStationsVeryWeak) {
    EXPECT_TRUE(handover::isA3ConditionMet(-115.0, -120.0, 3.0));
}

TEST(A3EventTest, BothStationsVeryStrong) {
    EXPECT_TRUE(handover::isA3ConditionMet(-30.0, -35.0, 3.0));
}

TEST(A3EventTest, NegativeHysteresis) {
    EXPECT_TRUE(handover::isA3ConditionMet(-72.0, -70.0, -5.0));
}
