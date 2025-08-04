#pragma once

#include <Job.h>

#include "../Synthetic/AllocJob.h"
#include "../Synthetic/NoOpJob.h"
#include "../Synthetic/RandomBranchingJob.h"
#include "../Synthetic/SleepJob.h"
#include "../Synthetic/SpinJob.h"

#include <memory>
#include <vector>

void createDefaultJobPool(std::vector<std::unique_ptr<Job>>& dest) {
    dest.push_back(std::make_unique<AllocJob>(1024 * 16));
    dest.push_back(std::make_unique<NoOpJob>());
    dest.push_back(std::make_unique<RandomBranchingJob>());
    dest.push_back(std::make_unique<SleepJob>(1));
    dest.push_back(std::make_unique<SpinJob>(1));
}