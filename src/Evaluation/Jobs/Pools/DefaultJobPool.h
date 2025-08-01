#pragma once

#include <Job.h>

#include "../Synthetic/AllocJob.h"
#include "../Synthetic/NoOpJob.h"
#include "../Synthetic/RandomBranchingJob.h"
#include "../Synthetic/SleepJob.h"
#include "../Synthetic/SpinJob.h"

#include <memory>
#include <vector>

inline std::vector<std::unique_ptr<Job>> createDefaultJobPool() {
    std::vector<std::unique_ptr<Job>> jobs;
    jobs.reserve(2);
    jobs.push_back(std::make_unique<AllocJob>(1024));
    jobs.push_back(std::make_unique<NoOpJob>());
    jobs.push_back(std::make_unique<RandomBranchingJob>());
//    jobs.push_back(std::make_unique<SleepJob>(1));
//    jobs.push_back(std::make_unique<SpinJob>(1));
    return jobs;
}