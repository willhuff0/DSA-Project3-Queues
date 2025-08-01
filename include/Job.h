#pragma once

class Job {
public:
    virtual ~Job() = default;
    virtual void operator()() = 0;
};