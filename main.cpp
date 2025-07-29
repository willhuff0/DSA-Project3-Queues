#include "src/Queues/OLD-BoundedRingBufferQueue.h"
#include "src/Evaluation/Job.h"
#include "src/Evaluation/JobSystem.h"

#include <iostream>
#include <functional>

int main() {
    OLD<Job*> queue(64);

    std::vector<Job> availableJobs {

    };

    JobSystem<OLD<Job*>> jobSystem(availableJobs);

    return 0;
}
