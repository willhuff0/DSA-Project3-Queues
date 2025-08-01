#pragma once

#include <IQueue.h>

#include <queue>

template<typename T>
class StdQueueUnsafe : public IQueue<T> {
public:
    void Enqueue(const T& value);
    bool Dequeue(T& out);

private:
    std::queue<T> queue;
};

template<typename T>
void StdQueueUnsafe<T>::Enqueue(const T& value) {
    queue.push(value);
}

template<typename T>
bool StdQueueUnsafe<T>::Dequeue(T& out) {
    if (queue.empty()) return false;
    out = queue.front();
    queue.pop();
    return true;
}

