#pragma once

struct Job {
    virtual void operator()() = 0;
};