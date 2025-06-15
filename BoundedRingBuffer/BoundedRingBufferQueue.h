#pragma once

#include <atomic>
#include <cassert>

template<class T>
class BoundedRingBufferQueue {
    struct Cell {
        std::atomic<size_t> sequence;
        T data;
    };

    const size_t bufferMask;
    Cell* const buffer;

    // Padding to avoid false sharing (https://en.wikipedia.org/wiki/False_sharing)
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueuePos{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeuePos{0};

public:
    BoundedRingBufferQueue(size_t bufferSize);
    ~BoundedRingBufferQueue();

    bool Enqueue(const T& value);
    bool Dequeue(T& out);
};

template<typename T>
BoundedRingBufferQueue<T>::BoundedRingBufferQueue(size_t bufferSize)
    : bufferMask(bufferSize - 1),
    buffer(reinterpret_cast<Cell*>(operator new[](sizeof(Cell) * bufferSize))) {
    assert((bufferSize & (bufferSize - 1)) == 0 && "bufferSize must be a power of two");

    for(size_t i = 0; i < bufferSize; i++) {
        new(&buffer[i]) Cell();
        buffer[i].sequence.store(i, std::memory_order_relaxed);
    }
}

template<typename T>
BoundedRingBufferQueue<T>::~BoundedRingBufferQueue() {
    operator delete[](buffer);
}

template<typename T>
bool BoundedRingBufferQueue<T>::Enqueue(const T &value) {
    Cell* cell;
    size_t pos = enqueuePos.load(std::memory_order_relaxed);

    while (true) {
        // since bufferSize is po2, pos & bufferMask == pos % bufferSize
        cell = &buffer[pos & bufferMask];
        size_t sequence = cell->sequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)sequence - (intptr_t)pos;

        if (diff == 0) {
            // Slot is free
            if (enqueuePos.compare_exchange_weak(pos, pos+1,
                                                 std::memory_order_relaxed, std::memory_order_relaxed)) {
                break;
            }
            // compare and exchange failed, another thread already swapped it, try again
        }
        else if (diff < 0) {
            // buffer is full
            return false;
        }
        else {
            // different thread won the enqueue, try again
            pos = enqueuePos.load(std::memory_order_relaxed);
        }
    }

    // got a cell
    cell->data = value;

    // mark cell ready for dequeue
    cell->sequence.store(pos + 1, std::memory_order_release);

    return true;
}

template<typename T>
bool BoundedRingBufferQueue<T>::Dequeue(T &out) {
    Cell* cell;
    size_t pos = dequeuePos.load(std::memory_order_relaxed);

    while (true) {
        cell = &buffer[pos & bufferMask];
        size_t sequence = cell->sequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)sequence - (intptr_t)(pos + 1);

        if (diff == 0) {
            // slot is ready to dequeue
            if (dequeuePos.compare_exchange_weak(pos, pos+1,
                                                 std::memory_order_relaxed, std::memory_order_relaxed)) {
                break;
            }
            // compare and exchange failed, another thread beat us, try again
        }
        else if (diff < 0) {
            // queue is empty
            return false;
        }
        else {
            // another thread beat us, try again
            pos = dequeuePos.load(std::memory_order_relaxed);
        }
    }

    // got a cell
    out = cell->data;

    // mark cell ready for enqueue
    cell->sequence.store(pos + 1, std::memory_order_release);

    return true;
}