#pragma once

#include <Job.h>

#include <thread>

/// Simulates IO latency or other blocking operations
class SleepJob : public Job {
public:
    // Constructor declaration
    SleepJob(int microseconds) : microseconds(microseconds) { }

    inline void operator()() override {
        std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
    }

private:
    const int microseconds;
};