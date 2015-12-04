#pragma once

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <string>

class Timer
{
public:
    Timer() 
    : begin(Clock::now()) 
    {
    }

    double ElapsedSeconds() const 
    {
        return std::chrono::duration_cast<Second>(Clock::now() - begin).count();
    }

private:
    using Clock = std::chrono::high_resolution_clock;
    using Second = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<Clock> begin;
};

inline void RuntimeError(const std::string& message)
{
    std::cout << "runtime error: " << message;
    exit(-1);
}

inline void ValidationError(const std::string& message)
{
    std::cout << "validation error: " << message;
    exit(-2);
}

inline std::string JoinPath(std::string path1, std::string path2)
{
    if (!path1.empty() && (path1.back() == '/' || path1.back() == '\\'))
        path1 = path1.substr(0, path1.length() - 1);

    if (!path2.empty() && (path2[0] == '/' || path2[0] == '\\'))
        path2 = path2.substr(1, path2.length() - 1);

    return path1 + '/' + path2;
}
