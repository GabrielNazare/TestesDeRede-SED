#include <gtest/gtest.h>
#include "SignalFilter.h"


TEST(SignalFilterTest, EmptyFilterThrowsException) {
    SignalFilter filter(5);
    EXPECT_THROW(filter.getFilteredRssi(), std::runtime_error);
}

TEST(SignalFilterTest, AverageOfIdenticalValues) {
    SignalFilter filter(5);
    filter.addSample(-70.0);
    filter.addSample(-70.0);
    filter.addSample(-70.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -70.0, 0.001);
}

TEST(SignalFilterTest, AverageOfVariedValues) {
    SignalFilter filter(4);
    filter.addSample(-80.0);
    filter.addSample(-60.0);
    filter.addSample(-70.0);
    filter.addSample(-70.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -70.0, 0.001);
}

TEST(SignalFilterTest, CircularBufferOverwriteBehavior) {
    SignalFilter filter(3);
    filter.addSample(-50.0);
    filter.addSample(-50.0);
    filter.addSample(-50.0);
    filter.addSample(-80.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -60.0, 0.001);
}

TEST(SignalFilterTest, FilterStateReset) {
    SignalFilter filter(5);
    filter.addSample(-72.0);
    filter.addSample(-68.0);
    filter.reset();
    EXPECT_EQ(filter.getSampleCount(), 0u);
    EXPECT_THROW(filter.getFilteredRssi(), std::runtime_error);
}

TEST(SignalFilterTest, ZeroWindowSizeThrowsException) {
    EXPECT_THROW(SignalFilter filter(0), std::invalid_argument);
}

TEST(SignalFilterTest, SignalNoiseSmoothing) {
    SignalFilter filter(5);
    filter.addSample(-70.0);
    filter.addSample(-70.0);
    filter.addSample(-40.0);
    filter.addSample(-70.0);
    filter.addSample(-70.0);
    double filtered = filter.getFilteredRssi();
    EXPECT_LT(filtered, -40.0);
    EXPECT_GT(filtered, -70.0);
}

TEST(SignalFilterTest, SingleSampleReturnsExactValue) {
    SignalFilter filter(10);
    filter.addSample(-85.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -85.0, 0.001);
    EXPECT_EQ(filter.getSampleCount(), 1u);
}

TEST(SignalFilterTest, WindowSizeOfOne) {
    SignalFilter filter(1);
    filter.addSample(-60.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -60.0, 0.001);
    filter.addSample(-90.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -90.0, 0.001);
}

TEST(SignalFilterTest, LargeWindowPartiallyFilled) {
    SignalFilter filter(100);
    filter.addSample(-50.0);
    filter.addSample(-60.0);
    EXPECT_NEAR(filter.getFilteredRssi(), -55.0, 0.001);
    EXPECT_EQ(filter.getSampleCount(), 2u);
}
