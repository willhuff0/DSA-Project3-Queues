#pragma once

#include <atomic>

/// For testing pure overhead of queue
struct NoOpJob {
    static std::atomic<size_t> counter;
    void operator()() {
        counter++;
    }
};