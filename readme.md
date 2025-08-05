## Purpose

This project benchmarks various lock-free Multi-Producer Multi-Consumer (MPMC) queue implementations for use in a job system. Job systems are fundamental to high-performance, multithreaded applications like web servers, game engines, scientific computing apps, and trading bots, where the efficiency of the underlying queue is critical.

The job queue is the backbone of a job system. An efficient queue can increase overall CPU utilization, which translates to better application performance (e.g., higher frame rates in games, faster simulations, or quicker data processing). This investigation provides empirical data on queue throughput and latency under various producer/consumer loads.

## Running the Code

1. Open [src/Config.h](src/Config.h):
    - Enable/Disable Throughput and Latency benchmarks by commenting out the corresponding defines.
    - Configure the BenchmarkSuiteConfig if desired.
    - **Ensure your CPU has at least as many threads as the largest producerCount + largest consumerCount**
      - We collected our data on a machine with 16 threads and 8 cores and found that our data was consistent even with slight contention with the OS.
    - Set both result paths (if using CLion the defaults should already work). It should point to a valid file path for creation; an extension is not necessary.
2. Build the queues executable with CMake.
3. Close all unnecessary processes on your machine (or don't) and run the executable from a terminal. Change the process priority to High for the most accurate results on Windows.
4. By default, results will be placed in [reporting/results](reporting/results).
5. If desired, use these Python scripts to generate plots:
```
python generate_throughput_plots.py
python generate_latency_plots.py
```

## Terms

- **Multi-Producer, Multi-Consumer**: Multiple threads are adding and removing items from the queue simultaneously.
- **Single-Producer, Multi-Consumer**: One thread adds items, while multiple threads remove them.
- **Lock-Free**: An algorithm is lock-free if it guarantees that at least one thread will make progress, regardless of the state of other threads. This avoids deadlocks and performance bottlenecks of locks (mutexes).
- **Blocking**: An operation is blocking if it can cause the calling thread to suspend its execution. Commonly, when a thread tries to acquire a lock that is already held, it blocks until the lock is released.
- **Bounded vs. Unbounded Queue**:
   - A **bounded** queue has a fixed, pre-allocated capacity. Enqueue operations will fail if the queue is full.
   - An **unbounded** queue can grow dynamically as needed. Enqueue operations typically involve memory allocation.
- **CAS (Compare-And-Swap)**: An atomic instruction that compares the value of a memory location with an expected value, and if they are equal, replaces it with a new value. This is a fundamental building block for many lock-free algorithms, allowing threads to modify shared data without locks by retrying the operation if another thread has intervened.
- **Job System**: A framework that manages a pool of worker threads to execute tasks (jobs) asynchronously. It typically uses a queue to distribute jobs among the threads, allowing for parallel processing.

## Queues Tested

###	Bounded Circular Buffer

A fixed-size ring buffer with atomic counters for head/tail and per-slot metadata. \
Article: https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue \
Implementation: [BoundedCircularBuffer.h](src/Queues/BoundedCircularBuffer.h)

### Unbounded Linked List (Michael & Scott)

Classic unbounded queue with linked nodes. Each enqueue/dequeue is a CAS on pointers. \
Paper: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf \
Implementation: [LinkedListQueue.h](src/Queues/LinkedListQueue.h)

### Others

In addition to the primary queues being tested, we included a few extras for reference: 
- [std::queue (Blocking)](src/Queues/StdQueueBlocking.h) uses std::queue guarded by a mutex.
- [moodycamel::ConcurrentQueue](src/Queues/ThirdParty/MoodycamelQueue.h) is a popular public library. See the [GitHub](https://github.com/cameron314/concurrentqueue).

## Synthetic Jobs

- [NoOpJob](src/Evaluation/Jobs/Synthetic/NoOpJob.h) - This job only increments an atomic counter. It can be used to measure pure queue overhead without any job execution cost.
- [AllocJob](src/Evaluation/Jobs/Synthetic/AllocJob.h) - Performs memory allocation and immediate deallocation of a specified size. This simulates memory-intensive operations common in real applications and tests how queue performance is affected when jobs stress the memory allocator.
- [SpinJob](src/Evaluation/Jobs/Synthetic/SpinJob.h) - Performs busy-waiting (spinning) for a precise duration using high-resolution timers. This simulates CPU-intensive work with predictable timing.
- [SleepJob](src/Evaluation/Jobs/Synthetic/SleepJob.h) - Uses std::this_thread::sleep_for() to simulate I/O latency or blocking operations. This represents jobs that yield the CPU and test how queues are affected in scenarios where threads become idle.
- [RandomBranchingJob](src/Evaluation/Jobs/Synthetic/RandomBranchingJob.h) - Creates unpredictable branch patterns using random number generation. This simulates applications with poor branch prediction, testing queue performance when the CPU pipeline is frequently stalled.

### Limitations

Synthetic jobs cannot fully stress queues in realistic ways for several reasons:

- Resource Contention: Real jobs compete for shared resources like file handles, network connections, database locks, etc. Our synthetic jobs only stress one dimension (CPU, memory, or time).
- Cache Behavior: Real applications have complex memory access patterns and data sharing between jobs. How the processor handles memory access, instruction scheduling, and cache usage during the execution of real-world jobs varies, which can indirectly impact the performance of the queue.
- Scheduling: Some real job systems often have priorities and dependencies. The test system uses simple round-robin job distribution without these complexities.

## Procedures & Results

The benchmarks are executed by the main program (`src/main.cpp`), which uses the `Benchmark` class (`src/Evaluation/Benchmark.h`) to run the tests.

1. **Configuration**: The benchmarks are run for a variety of configurations, including different numbers of producers, consumers, and total jobs to process.
2. **Execution**: For each queue type, the `Benchmark` class creates a `JobSystem` that spawns the specified number of producer and consumer threads.
3. **Data Collection**: The `JobSystem` measures either throughput or latency.
4. **Reporting**: The results are written to CSV files in the `reporting/results` directory.
5. **Visualization**: Python scripts (`reporting/generate_*.py`) process the CSV files to generate the plots displayed below.

PC used for our run: AMD 7700X (8-cores, 16-threads), 32 GB Memory, Windows 11 24H2 with minimal background processes.

### Throughput

Throughput is the number of elements (jobs) per second that can be moved through the queue.

**Procedure:**
1. For each test configuration, a `JobSystem` is initialized.
2. A stopwatch is started.
3. The `JobSystem`'s worker threads begin producing and consuming jobs.
4. The system waits for a predefined number of jobs to be processed.
5. The stopwatch is stopped, and the total elapsed time is recorded.
6. Throughput is calculated as `(total jobs processed) / (elapsed time)`.

|                      | Mutli-Producer, Multi-Consumer                                                                                                                                                            | Single-Producer, Multi-Consumer                                                                                                                                           |
|----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Excluding std::queue | ![producer_consumer_count_vs_throughput_per_thread_10 M_no_std_queue.png](reporting/plots/throughput_all_queues/producer_consumer_count_vs_throughput_per_thread_10%20M_no_std_queue.png) | ![consumer_count_vs_throughput_per_thread_10 M_no_std_queue.png](reporting/plots/throughput_one_producer/consumer_count_vs_throughput_per_thread_10%20M_no_std_queue.png) |
| Including std::queue | ![producer_consumer_count_vs_throughput_per_thread_10 M.png](reporting/plots/throughput_all_queues/producer_consumer_count_vs_throughput_per_thread_10%20M.png)                           | ![consumer_count_vs_throughput_per_thread_10 M.png](reporting/plots/throughput_one_producer/consumer_count_vs_throughput_per_thread_10%20M.png)                           |

### Latency

Latency is the time it takes for an element to go from being enqueued to being dequeued (time spent waiting in the queue).

**Procedure:**
1. The `JobSystem` is configured with latency tracking enabled.
2. When a producer enqueues a job, a timestamp is stored within the job.
3. When a consumer dequeues a job, another timestamp is taken.
4. The latency for that job is the difference between the two timestamps.
5. This is repeated for a predefined number of jobs, and the latencies are collected.
6. The results show the average latency across all processed jobs.

| Mutli-Producer, Multi-Consumer                                                                                                     | Single-Producer, Multi-Consumer                                                                                    |
|------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------|
| ![producer_consumer_count_vs_latency_100 K.png](reporting/plots/latency_all_queues/producer_consumer_count_vs_latency_100%20K.png) | ![consumer_count_vs_latency_100 K.png](reporting/plots/latency_one_producer/consumer_count_vs_latency_100%20K.png) |


## Analysis

### Key Findings

The benchmark results reveal significant performance differences between queue implementations across different concurrency scenarios and metrics:

#### Throughput Performance

**Multi-Producer, Multi-Consumer Scenario:**
- **Circular Buffer Queues** demonstrate the highest throughput, with both 4-cell and 16-cell variants achieving ~315,000 jobs/sec/thread at 2 threads
- **Linked List Queue** performs similarly well, reaching ~308,000 jobs/sec/thread at 2 threads
- **moodycamel::ConcurrentQueue** shows competitive performance but slightly lower peak throughput
- **std::queue (Blocking)** significantly underperforms, degrading rapidly with increased concurrency

**Single-Producer, Multi-Consumer Scenario:**
- All lock-free implementations show dramatically improved performance in this scenario
- **Circular Buffer Queues** excel with peak performance of ~552,000 jobs/sec/thread at 6 consumers
- **Linked List Queue** achieves similar performance (~539,000 jobs/sec/thread at 4 consumers)
- **moodycamel::ConcurrentQueue** reaches ~521,000 jobs/sec/thread
- The single-producer scenario eliminates producer contention, allowing consumer threads to scale more effectively

#### Scalability

- **Lock-free implementations** (Circular Buffer, Linked List, moodycamel) show good scalability up to 4-6 threads before performance plateaus
- **std::queue** exhibits poor scalability, with throughput degrading significantly as thread count increases
- **Contention patterns**: Multi-producer scenarios show earlier performance degradation due to producer-side contention

#### Latency Performance

**Multi-Producer, Multi-Consumer:**
- **Circular Buffer Queues** provide exceptionally low latency (300-700ns) across all thread counts
- **Linked List Queue** shows high single-thread latency (~1,200ns) but improves significantly with more threads
- **moodycamel::ConcurrentQueue** exhibits variable latency with a concerning spike at low thread counts
- **std::queue** maintains a consistent high latency (~1,250ns) at high thread counts

**Single-Producer, Multi-Consumer:**
- **Circular Buffer Queues** achieve the lowest latency (118-210ns), demonstrating excellent cache efficiency
- **Linked List Queue** provides good latency (~350-600ns) with minimal variation
- **moodycamel::ConcurrentQueue** shows moderate latency (~300-1,700ns)
- **std::queue** latency increases linearly with consumer count (590-2,650ns)

### Insights

#### Circular Buffer Superiority
The circular buffer implementation consistently outperform other queues in both throughput and latency metrics. Notes:
- **Cache efficiency**: Its contiguous memory layout reduces cache misses
- **No allocation**: Bounded memory usage prevents allocation overhead
- **Buffer size impact**: Both 4-cell and 16-cell variants perform similarly, suggesting the workload doesn't heavily stress buffer capacity

#### Lock-Free vs. Locking Trade-offs
- **Lock-free implementations** (Circular Buffer, Linked List, moodycamel) consistently outperform the mutex-based std::queue
- **Scalability**: Lock-free designs show better thread scalability, particularly in single-producer scenarios
- **Latency predictability**: Lock-free implementations generally provide more consistent latency characteristics

### Conclusions

The benchmarks demonstrate that lock-free queue implementations significantly outperform traditional mutex-based approaches. The circular buffer implementation emerges as the top performer across most metrics. The choice between circular buffer sizes (4 vs 16 cells) appears to have minimal impact for the tested workloads, suggesting that smaller buffers may be sufficient for many use cases. This result would likely be different if the tests were run with more threads since contention would increase.

For production systems, the circular buffer queue implementation would be the recommended choice for most scenarios, with the linked list queue serving as an alternative where unbounded capacity is required.
