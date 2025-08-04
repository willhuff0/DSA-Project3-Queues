#include "Evaluation/Benchmark.h"

#include "Queues/BoundedRingBufferQueue-Old.h"
#include "Queues/LinkedListQueue.h"
#include "Queues/BoundedCircularBuffer.h"
#include "Queues/StdQueueBlocking.h"

#include <fstream>

template<typename... Args>
void writeCsv(const std::string& path, const std::array<std::string, sizeof...(Args)>& header, const std::vector<std::tuple<Args...>>& rows) {
    std::ofstream file(path.c_str());

    for(const std::string& headerVal : header) {
        file << headerVal << ',';
    }
    file << std::endl;

    for(const std::tuple<Args...>& row : rows) {
        std::apply([&](const auto&... elements) {
            ((file << elements << ','), ...);
        }, row);
        file << std::endl;
    }

    file.close();
}

template<typename... TQueues>
void runThroughput(const std::string& path) {
    std::cout << "[Throughput] Starting throughput benchmark" << std::endl;

    static std::vector<size_t> jobCounts { (size_t)1E5, (size_t)2.5E5, (size_t)5E5, (size_t)7.5E5,
                                           (size_t)1E6, (size_t)2.5E6, (size_t)5E6, (size_t)7.5E6,
                                           (size_t)1E7 };
    static std::vector<int> producerCounts { 8 };
    static std::vector<int> consumerCounts { 8 };
    static size_t numIterations = jobCounts.size() * producerCounts.size() * consumerCounts.size();

    std::vector<std::tuple<const char* /*queueName*/, size_t /*jobCount*/, int /*producerCount*/, int /*consumerCount*/, size_t /*numJobsCompleted*/, double /*elapsedSeconds*/>> rows;
    rows.reserve(numIterations * sizeof...(TQueues));
    ([&](auto* ptr) {
        size_t i = 1;
        std::cout << "[Throughput] Queue: " << typeid(std::remove_reference_t<decltype(*ptr)>).name() << std::endl;
        for (size_t jobCount : jobCounts) {
            for (int producerCount : producerCounts) {
                for (int consumerCount: consumerCounts) {
                    std::cout << "[Throughput]   Iteration " << i++ << " / " << numIterations << std::endl;

                    Benchmark<std::remove_reference_t<decltype(*ptr)>> benchmark;
                    auto result = benchmark.RunThroughput(jobCount, producerCount, consumerCount);

                    auto numJobsCompleted = result.numJobsCompleted;
                    std::chrono::duration<double, std::chrono::seconds::period> elapsedSeconds = result.elapsed;
                    rows.emplace_back(typeid(std::remove_reference_t<decltype(*ptr)>).name(), jobCount, producerCount, consumerCount, numJobsCompleted, elapsedSeconds.count());
                }
            }
        }
    }(static_cast<TQueues*>(nullptr)), ...);

    static std::array<std::string, 6> header {
        "Queue",
        "Job Count",
        "Producer Count",
        "Consumer Count",
        "Num Jobs Completed",
        "Elapsed Seconds",
    };
    writeCsv(path, header, rows);
}

void runLatency(const std::string& path) {

}

int main() {
    runThroughput<LinkedListQueue<Job*>, BoundedCircularBuffer<Job*, 64>, StdQueueBlocking<Job*>>("Results/Throughput.csv");
    //runLatency<LinkedListQueue<Job*>, BoundedCircularBuffer<Job*, 64>, StdQueueBlocking<Job*>>("Results/Latency.csv");
    return 0;
}
