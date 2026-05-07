#include "SignalFilter.h"
#include <stdexcept>
#include <numeric>

SignalFilter::SignalFilter(size_t windowSize)
    : maxWindowSize(windowSize), writeIndex(0), currentSampleCount(0)
{
    if (windowSize == 0) {
        throw std::invalid_argument("Window size must be greater than zero.");
    }
    circularBuffer = new double[maxWindowSize]();
}

SignalFilter::~SignalFilter() {
    delete[] circularBuffer;
}

void SignalFilter::addSample(double rssiSample) {
    circularBuffer[writeIndex] = rssiSample;
    writeIndex = (writeIndex + 1) % maxWindowSize;

    if (currentSampleCount < maxWindowSize) {
        ++currentSampleCount;
    }
}

double SignalFilter::getFilteredRssi() const {
    if (currentSampleCount == 0) {
        throw std::runtime_error("No samples available in filter.");
    }

    double rssiSum = 0.0;
    for (size_t i = 0; i < currentSampleCount; ++i) {
        rssiSum += circularBuffer[i];
    }

    return rssiSum / static_cast<double>(currentSampleCount);
}

void SignalFilter::reset() {
    writeIndex = 0;
    currentSampleCount = 0;
    for (size_t i = 0; i < maxWindowSize; ++i) {
        circularBuffer[i] = 0.0;
    }
}
