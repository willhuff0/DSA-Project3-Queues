#pragma once

#include <Job.h>

#include <atomic>

/// For testing pure overhead of queue
class NoOpJob : public Job {
public:
    NoOpJob() = default;
    ~NoOpJob() override = default;

    inline void operator()() override {
        counter++;
    }

private:
    std::atomic<size_t> counter{};
};