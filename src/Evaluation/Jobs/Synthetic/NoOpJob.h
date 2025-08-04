#pragma once

#include <Job.h>

#include <atomic>

/// Simulates pure overhead of queue
class NoOpJob : public Job {
public:
    // Constructor and Destructor declaration
    NoOpJob() = default;
    ~NoOpJob() override = default;

    inline void operator()() override {
        counter++;
    }

private:
    std::atomic<size_t> counter{};
};