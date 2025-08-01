#pragma once

#include <Job.h>

class AllocJob : public Job {
public:
    explicit AllocJob(size_t size) : size(size) { }
    ~AllocJob() override = default;

    inline void operator()() override {
        void* ptr = operator new(size);
        operator delete(ptr);
    }

private:
    const size_t size;
};