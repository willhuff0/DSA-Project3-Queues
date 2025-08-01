#pragma once

#include <Job.h>

#include <chrono>

/// Simulates CPU load with precise timing
struct SpinJob : public Job {
public:
    SpinJob(int microseconds) : microseconds(microseconds) { }

    inline void operator()() override {
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() < microseconds) {
            // Spin
        }
    }

private:
    const int microseconds;
};