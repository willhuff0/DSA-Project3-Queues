#pragma once

#include <Job.h>

#include <cstddef>

class AllocJob : public Job {
public:
    // Constructor and Destructor declaration
    explicit AllocJob(size_t size) : size(size) { }
    ~AllocJob() override = default;

    // Operator() override
    inline void operator()() override {
        void* ptr = operator new(size);
        operator delete(ptr);
    }

private:
    const size_t size;
};