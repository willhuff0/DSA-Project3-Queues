#pragma once

#include <thread>

/// Simulates IO latency or other blocking operations
struct SleepJob {
    int microseconds;
    void operator()() const {
        std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
    }
};