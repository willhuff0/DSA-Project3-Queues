#include "Evaluation/JobSystem.h"
#include "Evaluation/Jobs/Pools/DefaultJobPool.h"

#include "Queues/OLD-BoundedRingBufferQueue.h"
#include "Queues/StdQueueUnsafe.h"
#include "Queues/StdQueueBlocking.h"

int main() {
    auto defaultJobPool = createDefaultJobPool();

    {
        JobSystem<OLD<Job*>> jobSystem(defaultJobPool);
        jobSystem.RunTest(4, 4, 10E6);
    }
    {
        JobSystem<StdQueueBlocking<Job*>> jobSystem(defaultJobPool);
        jobSystem.RunTest(4, 4, 10E6);
    }

    return 0;
}
