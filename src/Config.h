#pragma once

#include <vector>

#define ENABLE_THROUGHPUT_BENCHMARK
#define THROUGHPUT_CONFIG defaultThroughputConfig
#define THROUGHPUT_BASEPATH "../reporting/results/throughput/throughput"

#define ENABLE_LATENCY_BENCHMARK
#define LATENCY_CONFIG defaultLatencyConfig
#define LATENCY_BASEPATH "../reporting/results/latency/latency"

struct BenchmarkSuiteConfig {
    int iterations;
    std::vector<size_t> jobCounts;
    std::vector<int> producerCounts;
    std::vector<int> consumerCounts;
};

static BenchmarkSuiteConfig defaultThroughputConfig {
    6,                                         // iterations
    { (size_t)1E6, (size_t)5E6, (size_t)1E7 }, // jobCounts
    { 1, 2, 4, 6, 8 },                         // producerCounts
    { 1, 2, 4, 6, 8 },                         // consumerCounts
};

static BenchmarkSuiteConfig defaultLatencyConfig {
    6,                                         // iterations
    { (size_t)1E5, (size_t)5E5, (size_t)1E6 }, // jobCounts
    { 1, 2, 4, 6, 8 },                         // producerCounts
    { 1, 2, 4, 6, 8 },                         // consumerCounts
};
