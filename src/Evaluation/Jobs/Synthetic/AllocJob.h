#pragma once

struct AllocJob {
    int size;
    void operator()() {
        void* ptr = operator new(size);
        operator delete(ptr);
    }
};