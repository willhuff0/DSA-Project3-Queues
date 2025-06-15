#include "BoundedRingBuffer/BoundedRingBufferQueue.h"

#include <iostream>

int main() {
    BoundedRingBufferQueue<int> queue(64);

    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);
    queue.Enqueue(4);
    queue.Enqueue(5);

    int val;
    while (queue.Dequeue(val)) {
        std::cout << val << std::endl;
    }

    return 0;
}
