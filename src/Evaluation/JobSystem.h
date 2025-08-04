#pragma once

#include <Job.h>
#include <IQueue.h>

#include <iostream>
#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename QueueT, bool measureLatency>
class JobSystem {
    static_assert(std::is_base_of_v<IQueue<Job*>, QueueT>);
public:
    explicit JobSystem(const std::vector<std::unique_ptr<Job>>& jobs) : availableJobs(jobs) { }

    ~JobSystem() {
        if (running) {
            StopWorkers();
        }
    }

    void StartWorkers(int numProducers, int numConsumers) {
        running = true;
        numJobsCompleted = 0;
        if constexpr (measureLatency) latenciesCumulative.clear();

        threads.reserve(numProducers + numConsumers);
        for (int i = 0; i < numProducers; i++) {
            threads.emplace_back([this, i] { producerEntry(i); });
        }
        for (int i = 0; i < numConsumers; i++) {
            threads.emplace_back([this] { consumerEntry(); });
        }
    }

    void StopWorkers() {
        running = false;
        cv.notify_all();
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    void WaitForJobs(size_t numJobs) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this, numJobs] {
            return numJobsCompleted >= numJobs || !running;
        });
    }

    [[nodiscard]] size_t GetCompletedJobCount() const {
        return numJobsCompleted.load();
    }

    [[nodiscard]] std::vector<std::chrono::high_resolution_clock::duration> GetLatencies() {
        std::lock_guard<std::mutex> lock(latenciesMutex);
        return latenciesCumulative;
    }

private:
    void producerEntry(int index) {
        int nextJobType = index % availableJobs.size();
        while (running) {
            Job* jobToInsert = availableJobs[nextJobType].get();
            if constexpr (measureLatency) jobToInsert->enqueueTime = std::chrono::high_resolution_clock::now();
            queue.Enqueue(jobToInsert);
            nextJobType = (nextJobType + 1) % availableJobs.size();
        }
    }

    void consumerEntry() {
        std::vector<std::chrono::high_resolution_clock::duration> latencies;

        Job* job;
        while (running) {
            if (queue.Dequeue(job)) {
                if constexpr (measureLatency) {
                    auto dequeueTime = std::chrono::high_resolution_clock::now();
                    auto latency = dequeueTime - job->enqueueTime;
                    latencies.push_back(latency);
                }

                job->operator()();
                numJobsCompleted++;
                cv.notify_one();
            }
        }

        if constexpr (measureLatency) {
            std::lock_guard lock(latenciesMutex);
            latenciesCumulative.insert(latenciesCumulative.end(), latencies.begin(), latencies.end());
        }
    }

private:
    QueueT queue;
    const std::vector<std::unique_ptr<Job>>& availableJobs;

    std::atomic<bool> running = false;
    std::atomic<size_t> numJobsCompleted = 0;

    std::vector<std::thread> threads;

    std::mutex mtx;
    std::condition_variable cv;

    std::vector<std::chrono::high_resolution_clock::duration> latenciesCumulative;
    std::mutex latenciesMutex;
};