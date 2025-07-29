#pragma once

#include "Job.h"
#include "../Queues/Queue.h"

#include <iostream>
#include <atomic>
#include <functional>
#include <thread>

template<typename QueueT>
class JobSystem {
    static_assert(std::is_base_of_v<Queue<Job>, QueueT>);
public:
    explicit JobSystem(std::vector<Job> jobs) : available_jobs(std::move(jobs)) { }

    void RunTest(int numProducers, int numConsumers) {
        numJobsCompleted = 0;
        for (int i = 0; i < numProducers; i++) {
            producers.emplace_back(producerEntry(i));
        }
        for (int i = 0; i < numConsumers; i++) {
            consumers.emplace_back(consumerEntry());
        }
        while (numJobsCompleted < 1000000) { }
        std::cout << "done" << std::endl;
    }

private:
    void producerEntry(int index) {
        int next_job_type = index % available_jobs.size();
        while (running) {
            Job* jobToInsert = &available_jobs[next_job_type];
            next_job_type = (next_job_type + 1) % available_jobs.size();
            queue.enqueue(jobToInsert);
        }
    }

    void consumerEntry() {
        Job* job;
        while (running) {
            if (!queue.dequeue(&job)) continue;
            job->operator()();
            numJobsCompleted++;
        }
    }

private:
    QueueT queue;

    std::vector<Job> available_jobs; // readonly once test starts

    std::atomic<bool> running = false;
    std::atomic<unsigned long long> numJobsCompleted = 0;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers; // workers
};