#pragma once

#include <chrono>

/// Simulates CPU load with precise timing
struct SpinJob {
    int microseconds;
    void operator()() {
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() < microseconds) {
            // Spin
        }
    }
};