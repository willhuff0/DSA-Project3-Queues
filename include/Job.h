#pragma once

#include <chrono>

struct Job {
    virtual ~Job() = default;
    virtual void operator()() = 0;

    std::chrono::high_resolution_clock::time_point enqueueTime;
};