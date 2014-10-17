#pragma once

#include <chrono>

class Timer {
public:
    Timer() 
        : begin_(clock_::now()) 
    {}
    double elapsed() const {
        return std::chrono::duration_cast<second_>(clock_::now() - begin_).count();
    }
private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> begin_;
};
