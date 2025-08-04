#pragma once

#include <IQueue.h>

#include "concurrentqueue.h"

#include <string>

template<typename T>
class MoodycamelQueue : public IQueue<T> {
public:
    static std::string GetName() { return "moodycamel::ConcurrentQueue"; }

    void Enqueue(const T& value) override;
    bool Dequeue(T& out) override;

private:
    moodycamel::ConcurrentQueue<T> queue;
};

template<typename T>
void MoodycamelQueue<T>::Enqueue(const T& value) {
    queue.enqueue(value);
}

template<typename T>
bool MoodycamelQueue<T>::Dequeue(T& out) {
    return queue.try_dequeue(out);
}
