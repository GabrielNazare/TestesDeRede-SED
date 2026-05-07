#pragma once
#include <vector>
#include <deque>
#include <numeric>
#include <stdexcept>

class SignalFilter {
public:
    explicit SignalFilter(size_t windowSize = 5);
    ~SignalFilter();

    SignalFilter(const SignalFilter&)            = delete;
    SignalFilter& operator=(const SignalFilter&) = delete;

    void addSample(double rssiSample);
    double getFilteredRssi() const;
    void reset();
    size_t getSampleCount() const { return currentSampleCount; }

private:
    double* circularBuffer;
    size_t  maxWindowSize;
    size_t  writeIndex;
    size_t  currentSampleCount;
};
