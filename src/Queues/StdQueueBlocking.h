#pragma once

#include <IQueue.h>

#include <mutex>
#include <queue>

template<typename T>
class StdQueueBlocking : public IQueue<T> {
public:
    void Enqueue(const T& value);
    bool Dequeue(T& out);

private:
    std::queue<T> queue;
    std::mutex mutex;
};

template<typename T>
void StdQueueBlocking<T>::Enqueue(const T& value) {
    std::lock_guard lock(mutex);
    queue.push(value);
}

template<typename T>
bool StdQueueBlocking<T>::Dequeue(T& out) {
    std::lock_guard lock(mutex);
    if (queue.empty()) return false;
    out = queue.front();
    queue.pop();
    return true;
}

