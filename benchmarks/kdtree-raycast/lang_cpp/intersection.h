#pragma once

#include <limits>

struct Intersection
{
    double rayT = std::numeric_limits<double>::infinity();
    double rayTEpsilon = 0.0;
};
