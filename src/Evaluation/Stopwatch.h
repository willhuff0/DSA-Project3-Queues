#pragma once

#include <iostream>
#include <chrono>

template <typename Clock = std::chrono::high_resolution_clock>
class Stopwatch {
public:
    Stopwatch() : last(Clock::now()) {}

    void Reset() {
        last = Clock::now();
    }

    [[nodiscard]] typename Clock::duration GetElapsed() const {
        return Clock::now() - last;
    }

    // Finds time difference between this Tick() call and last Tick() call
    typename Clock::duration Tick() {
        auto now = Clock::now();
        auto elapsed = now - last;
        last = now;
        return elapsed;
    }

    // Automatically calls Tick() and prints time difference between call of this function and last Tick() call
    typename Clock::duration TickAndPrint(const std::string& message = "") {
        auto elapsed = Tick();
        std::cout << message << " (" << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms)" << std::endl;
        return elapsed;
    }

private:
    typename Clock::time_point last;
};