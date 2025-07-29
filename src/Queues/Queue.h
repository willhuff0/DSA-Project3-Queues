#pragma once

template<typename T>
class Queue {
public:
    // Enqueues value.
    virtual void Enqueue(const T& value) = 0;

    // Dequeues a value and stores in out. Returns false if the queue is empty, otherwise true.
    virtual bool Dequeue(T& out) = 0;
};