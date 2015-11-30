#pragma once

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <string>

class Timer
{
public:
    Timer() 
    : begin_(clock_::now()) 
    {
    }

    double elapsed() const 
    {
        return std::chrono::duration_cast<second_>(clock_::now() - begin_).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> begin_;
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
