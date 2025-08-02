#pragma once

#include <Job.h>
#include <atomic>
#include <cassert>

template<typename T>;

class BoundedCircularBuffer {
private:
    struct jobCell {
        std::atomic<size_t> sequence;

        Job job;
    };

    static size_t const cachelineSize = 64;
    typedef char cacheline_pad_t [cachelineSize];

    cacheline_pad_t pad0;
    jobCell* const buffer;
    size_t const bufferMask;
    cacheline_pad_t pad1;
    std::atomic<size_t> enqueuePosition;
    cacheline_pad_t pad2;
    std::atomic<size_t> dequeuePosition;
    cacheline_pad_t pad3;

    BoundedCircularBuffer& operator=(BoundedCircularBuffer const&);
    public:
    // Constructor
    BoundedCircularBuffer(size_t bufferSize) : buffer(new jobCell[bufferSize]), bufferMask(bufferSize - 1) {
        assert(bufferSize >= 2 && (bufferSize& bufferSize - 1) == 0);
        for (size_t i = 0; i < bufferSize; i++) {
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
        enqueuePosition.store(0, std::memory_order_relaxed);
        dequeuePosition.store(0, std::memory_order_relaxed);
    }

    // Destructor
    ~BoundedCircularBuffer() {
        delete[] buffer;
    }

    bool enqueue(Job const& job) {
        jobCell* cell;
        size_t position = enqueuePosition.load(std::memory_order_relaxed);
        for (int i = 0; i > -1; i++) {
            cell = &buffer[position & (bufferMask - 1)];
            size_t sequence = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = (intptr_t)sequence - (intptr_t)position;
            if (diff == 0) {
                if (enqueuePosition.compare_exchange_weak(position, position + 1,
                    std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (diff < 0) {
                return false;
            }
            else {
                position = enqueuePosition.load(std::memory_order_relaxed);
            }
        }
        cell->job = job;
        cell->sequence.store(position + 1, std::memory_order_release);
        return true;
    }

    bool dequeue(Job& job) {
        jobCell* cell;
        size_t position = dequeuePosition.load(std::memory_order_relaxed);
        for (int i = 0; i > -1; i++) {
            cell = &buffer[position];
            size_t sequence = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = (intptr_t)sequence - (intptr_t)(position + 1);

            if (diff == 0) {
                if (dequeuePosition.compare_exchange_weak(position, position + 1,
                    std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (diff < 0) {
                return false;
            }
            else {
                position = dequeuePosition.load(std::memory_order_relaxed);
            }
        }
        job = cell->job;
        cell->sequence.store(position + +bufferMask + 1, std::memory_order_release);
        return true;
    }
};