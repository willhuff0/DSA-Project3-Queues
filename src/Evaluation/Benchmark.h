#pragma once

#include <Job.h>

#include "JobSystem.h"
#include "Jobs/Pools/DefaultJobPool.h"
#include "Stopwatch.h"

#include <string>
#include <memory>
#include <vector>
#include <chrono>

// Structures for throughput and latency outputs
struct ThroughputResult {
    size_t numJobsCompleted;
    std::chrono::high_resolution_clock::duration elapsed;
};

struct LatencyResult {
    std::vector<std::chrono::high_resolution_clock::duration> latencies;
};

template<typename QueueT>
class Benchmark {
public:
    // When Benchmark is initialized, if there are no jobs, it will call createDefaultJobPool to create jobs
    explicit Benchmark() {
        if (jobs.empty()) {
            createDefaultJobPool(jobs);
        }
    }

    // Finds the throughput of all the jobs
    ThroughputResult RunThroughput(size_t numJobs, int numProducers, int numConsumers) {
        // Initializes stopwatch object
        Stopwatch stopwatch;

        // Makes a job system
        auto jobSystem = std::make_unique<JobSystem<QueueT, false>>(jobs);
        jobSystem->StartWorkers(numProducers, numConsumers);

        stopwatch.Reset();
        jobSystem->WaitForJobs(numJobs);
        auto elapsed = stopwatch.Tick();

        jobSystem->StopWorkers();

        return {
            jobSystem->GetCompletedJobCount(),
            elapsed,
        };
    }

    // Finds the latency of all jobs
    LatencyResult RunLatency(size_t numJobs, int numProducers, int numConsumers) {
        auto jobSystem = std::make_unique<JobSystem<QueueT, true>>(jobs);
        jobSystem->StartWorkers(numProducers, numConsumers);
        jobSystem->WaitForJobs(numJobs);
        jobSystem->StopWorkers();

        return {
            jobSystem->GetLatencies(),
        };
    }


protected:
    inline static std::vector<std::unique_ptr<Job>> jobs;
};