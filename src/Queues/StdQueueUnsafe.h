#pragma once

#include <IQueue.h>

#include <queue>

template<typename T>
class StdQueueUnsafe : public IQueue<T> {
public:
    static std::string GetName() { return "std::queue (Unsafe)"; }

    // Function declarations for enqueue and dequeue
    void Enqueue(const T& value) override;
    bool Dequeue(T& out) override;

private:
    std::queue<T> queue;
};

// Function definition for enqueue
template<typename T>
void StdQueueUnsafe<T>::Enqueue(const T& value) {
    queue.push(value);
}

// Function definition for dequeue
template<typename T>
bool StdQueueUnsafe<T>::Dequeue(T& out) {
    if (queue.empty()) return false;
    out = queue.front();
    queue.pop();
    return true;
}

