#pragma once

#include <Job.h>

#include <atomic>
#include <random>

class RandomBranchingJob : public Job {
public:
    RandomBranchingJob() = default;

    inline void operator()() override {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> branch(0, 1);
        if (branch(rng)) {
            counter1++;
        } else {
            counter2++;
        }
    }

private:
    std::atomic<size_t> counter1{};
    std::atomic<size_t> counter2{};
};