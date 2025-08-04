#include "Evaluation/JobSystem.h"
#include "Evaluation/Jobs/Pools/DefaultJobPool.h"

#include "Queues/StdQueueUnsafe.h"
#include "Queues/StdQueueBlocking.h"

int main() {
    std::vector<std::unique_ptr<Job>> jobPool;
    createDefaultJobPool(jobPool);

    {
        JobSystem<StdQueueBlocking<Job*>> jobSystem(jobPool);
        jobSystem.RunTest(4, 4, 10E6);
    }

    return 0;
}
