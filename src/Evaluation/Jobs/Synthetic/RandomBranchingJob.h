#pragma once

#include <atomic>
#include <random>

struct RandomBranchingJob {
    static std::atomic<size_t> counter1;
    static std::atomic<size_t> counter2;

    void operator()() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> branch(0, 1);
        if (branch(rng)) {
            counter1++;
        } else {
            counter2++;
        }
    }
};