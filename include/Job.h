#pragma once

#include <chrono>

// Job structure
struct Job {
    virtual ~Job() = default;
    virtual void operator()() = 0;

    // Holds time stamp variable
    std::chrono::high_resolution_clock::time_point enqueueTime;
};