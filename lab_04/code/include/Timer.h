#pragma once
#include <chrono>

class Timer {
public:
    using clock = std::chrono::steady_clock;
    clock::time_point start;
    Timer() : start(clock::now()) {}
    void reset() { start = clock::now(); }
    long long ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count();
    }
    long long us() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - start).count();
    }
    long long ns() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - start).count();
    }
};
