#pragma once

#include "ray.h"
#include "vector.h"
#include <array>
#include <limits>

struct Triangle {
  std::array<Vector, 3> points;

  struct Intersection {
    double t = std::numeric_limits<double>::infinity();
    double epsilon = 0.0;
    double b1 = 0.0;
    double b2 = 0.0;
  };
};

bool IntersectTriangle(const Ray& ray, const Triangle& triangle,
                       Triangle::Intersection& intersection);
