#include "Evaluation/Benchmark.h"

#include "Queues/LinkedListQueue.h"
#include "Queues/BoundedCircularBuffer.h"
#include "Queues/ThirdParty/MoodycamelQueue.h"
#include "Queues/StdQueueBlocking.h"
#include "Config.h"

#include <fstream>
#include <filesystem>

// Returns a string that shows a number of jobs in shorthand
std::string formatJobCount(size_t count) {
    if (count >= 1000000000) {
        return std::to_string(count / 1000000000) + " B";
    }
    if (count >= 1000000) {
        return std::to_string(count / 1000000) + " M";
    }
    if (count >= 1000) {
        return std::to_string(count / 1000) + " K";
    }
    return std::to_string(count);
}

// Returns a string that is formatted for throughput
std::string formatThroughput(double throughput, int decimalPlaces) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimalPlaces);

    if (throughput >= 1000000000.0) {
        ss << (throughput / 1000000000.0);
        return ss.str() + " B";
    }
    if (throughput >= 1000000.0) {
        ss << (throughput / 1000000.0);
        return ss.str() + " M";
    }
    if (throughput >= 1000.0) {
        ss << (throughput / 1000.0);
        return ss.str() + " K";
    }

    ss << throughput;
    return ss.str();
}

// Attempts to create a path towards the data set
bool createDirectoriesRecursive(const std::string& path) {
    std::filesystem::path filePath = path;
    std::filesystem::path directoryPath = filePath.parent_path();

    try {
        if (!std::filesystem::exists(directoryPath)) {
            std::filesystem::create_directories(directoryPath);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Prints error message when unable to find the path
        std::cerr << "Error creating directories: " << e.what() << std::endl;
        return false;
    }

    return true;
}

template<typename... Args>
// Creates a .csv file consisting of the data
void writeCsv(const std::string& path, const std::array<std::string, sizeof...(Args)>& header, const std::vector<std::tuple<Args...>>& rows) {
    // Creates the path to where the csv file will be
    createDirectoriesRecursive(path);
    std::ofstream file(path.c_str());

    // Writes header to the file
    for(const std::string& headerVal : header) {
        file << headerVal << ',';
    }
    file << std::endl;

    // Writes contents of rows to the file
    for(const std::tuple<Args...>& row : rows) {
        std::apply([&](const auto&... elements) {
            ((file << elements << ','), ...);
        }, row);
        file << std::endl;
    }

    file.close();
}

// Runs tests and measures throughput, and outputs to console that throughput is being measured and the exact numbers
template<typename... TQueues>
void runThroughput(const BenchmarkSuiteConfig& config, const std::string& basepath) {
    for (const auto& jobCount : config.jobCounts) {
        std::cout << "============================================================" << std::endl;
        std::cout << "[Throughput] Running benchmark for " << formatJobCount(jobCount) << " jobs." << std::endl;
        std::cout << "============================================================" << std::endl;

        std::vector<std::tuple<std::string /*queueName*/, int /*threadCount*/, double /*avgThroughput*/>> rows;

        size_t totalTestConfigs = config.producerCounts.size() * sizeof...(TQueues);
        size_t testConfigI = 1;

        ([&](auto* ptr) {
            using QueueType = std::remove_reference_t<decltype(*ptr)>;
            std::cout << "[Throughput] Benchmarking Queue: " << QueueType::GetName() << std::endl;

            for (size_t i = 0; i < config.producerCounts.size(); ++i) {
                int producerCount = config.producerCounts[i];
                int consumerCount = config.consumerCounts[i];

                std::cout << "[Throughput]  Config: " << producerCount << "P" << consumerCount << "C (" << testConfigI++ << "/" << totalTestConfigs << ")" << std::endl;

                double totalThroughput = 0.0;
                for (int iteration = 1; iteration <= config.iterations; iteration++) {
                    std::cout << "[Throughput]   Iteration " << iteration << "/" << config.iterations << "...";
                    Benchmark<QueueType> benchmark;
                    auto result = benchmark.RunThroughput(jobCount, producerCount, consumerCount);

                    auto numJobsCompleted = result.numJobsCompleted;
                    std::chrono::duration<double, std::chrono::seconds::period> elapsedSeconds = result.elapsed;
                    auto throughput = numJobsCompleted / elapsedSeconds.count();
                    totalThroughput += throughput;
                    std::cout << " Throughput: " << formatThroughput(throughput, 3) << " jobs/second" << std::endl;
                }

                double avgThroughput = totalThroughput / config.iterations;
                double throughputPerThread = avgThroughput / (producerCount + consumerCount);
                std::cout << "[Throughput]  Average Throughput: " << formatThroughput(avgThroughput, 3) << " jobs/second" << std::endl;
                std::cout << "[Throughput]  Throughput per thread: " << formatThroughput(throughputPerThread, 3) << " jobs/second/thread" << std::endl;
                rows.emplace_back(QueueType::GetName(), std::max(producerCount, consumerCount), throughputPerThread);
            }
        }(static_cast<TQueues*>(nullptr)), ...);

        auto path = basepath + "_job_count_" + formatJobCount(jobCount) + ".csv";
        static std::array<std::string, 3> header{
                "Queue",
                "Producer/Consumer Count",
                "Average Throughput per Thread (jobs/sec/thread)"
        };
        writeCsv(path, header, rows);
        std::cout << "[Throughput] Saved results to " << path << std::endl;
    }
}

// Runs latency tests, and outputs to console what tests are being performed and the data being collected
template<typename... TQueues>
void runLatency(const BenchmarkSuiteConfig& config, const std::string& basepath) {
    for (const auto& jobCount : config.jobCounts) {
        std::cout << "============================================================" << std::endl;
        std::cout << "[Latency] Running benchmark for " << formatJobCount(jobCount) << " jobs." << std::endl;
        std::cout << "============================================================" << std::endl;

        std::vector<std::tuple<std::string /*queueName*/, int /*threadCount*/, double /*avgLatency*/>> rows;

        size_t totalTestConfigs = config.producerCounts.size() * sizeof...(TQueues);
        size_t testConfigI = 1;

        ([&](auto* ptr) {
            using QueueType = std::remove_reference_t<decltype(*ptr)>;
            std::cout << "[Latency] Benchmarking Queue: " << QueueType::GetName() << std::endl;

            for (size_t i = 0; i < config.producerCounts.size(); ++i) {
                int producerCount = config.producerCounts[i];
                int consumerCount = config.consumerCounts[i];

                std::cout << "[Latency]  Config: " << producerCount << "P" << consumerCount << "C (" << testConfigI++ << "/" << totalTestConfigs << ")" << std::endl;

                double totalAvgLatency = 0.0;
                for (int iteration = 1; iteration <= config.iterations; iteration++) {
                    std::cout << "[Latency]   Iteration " << iteration << "/" << config.iterations << "...";
                    Benchmark<QueueType> benchmark;
                    auto result = benchmark.RunLatency(jobCount, producerCount, consumerCount);

                    double sum_ns = 0;
                    for (const auto& l : result.latencies) {
                        sum_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(l).count();
                    }
                    double avg_ns = result.latencies.empty() ? 0 : sum_ns / result.latencies.size();

                    totalAvgLatency += avg_ns;
                    std::cout << " Avg Latency: " << avg_ns << " ns" << std::endl;
                }

                double avgLatency = totalAvgLatency / config.iterations;
                rows.emplace_back(QueueType::GetName(), std::max(producerCount, consumerCount), avgLatency);
                std::cout << "[Latency]  Average Latency: " << avgLatency << " ns" << std::endl;
            }
        }(static_cast<TQueues*>(nullptr)), ...);

        auto path = basepath + "_job_count_" + formatJobCount(jobCount) + ".csv";
        static std::array<std::string, 3> header{
                "Queue",
                "Producer/Consumer Count",
                "Average Latency (ns)"
        };
        writeCsv(path, header, rows);
        std::cout << "[Latency] Saved results to " << path << std::endl;
    }
}

int main() {
#if defined(ENABLE_THROUGHPUT_BENCHMARK)
    runThroughput<LinkedListQueue<Job*>, BoundedCircularBufferQueue<Job*, 4>, BoundedCircularBufferQueue<Job*, 16>, MoodycamelQueue<Job*>, StdQueueBlocking<Job*>>(THROUGHPUT_CONFIG, THROUGHPUT_BASEPATH);
#endif
#if defined(ENABLE_LATENCY_BENCHMARK)
    runLatency<LinkedListQueue<Job*>, BoundedCircularBufferQueue<Job*, 4>, BoundedCircularBufferQueue<Job*, 16>, MoodycamelQueue<Job*>, StdQueueBlocking<Job*>>(LATENCY_CONFIG, LATENCY_BASEPATH);
#endif
    return 0;
}
