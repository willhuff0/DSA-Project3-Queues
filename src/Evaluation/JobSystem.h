#pragma once

#include <Job.h>
#include <IQueue.h>

#include "Stopwatch.h"

#include <iostream>
#include <atomic>
#include <functional>
#include <thread>

template<typename QueueT>
class JobSystem {
    static_assert(std::is_base_of_v<IQueue<Job*>, QueueT>);
public:
    explicit JobSystem(const std::vector<std::unique_ptr<Job>>& jobs) : availableJobs(jobs) { }

    void RunTest(int numProducers, int numConsumers, size_t numJobs) {
        started = false;
        running = true;

        numJobsCompleted = 0;
        threads.reserve(numProducers + numConsumers);
        for (int i = 0; i < numProducers; i++) {
            threads.emplace_back([this, i] { producerEntry(i); });
        }
        for (int i = 0; i < numConsumers; i++) {
            threads.emplace_back([this] { consumerEntry(); });
        }

        std::cout << "Starting test [" << typeid(QueueT).name() << "]" << std::endl;
        Stopwatch stopwatch;
        started = true;

        while (numJobsCompleted < numJobs) { }

        stopwatch.TickAndPrint("Test result [" + std::string(typeid(QueueT).name()) + "]: completed " + std::to_string(numJobsCompleted) + " jobs.");
        running = false;

        for (auto& thread : threads) {
            thread.join();
        }
    }

private:
    void producerEntry(int index) {
        while (!started) { }

        int nextJobType = index % availableJobs.size();
        while (running) {
            Job* jobToInsert = availableJobs[nextJobType].get();
            nextJobType = (nextJobType + 1) % availableJobs.size();
            queue.Enqueue(jobToInsert);
        }
    }

    void consumerEntry() {
        while (!started) { }

        Job* job;
        while (running) {
            if (!queue.Dequeue(job)) continue;
            job->operator()();
            numJobsCompleted++;
        }
    }

private:
    QueueT queue;

    const std::vector<std::unique_ptr<Job>>& availableJobs;

    std::atomic<bool> started = false;
    std::atomic<bool> running = false;
    std::atomic<size_t> numJobsCompleted = 0;

    std::vector<std::thread> threads;
};