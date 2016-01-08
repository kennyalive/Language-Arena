#pragma once

#include "vector.h"
#include <cassert>
#include <cmath>

class Ray {
public:
  Ray(const Vector& origin_, const Vector& direction_)
      : origin(origin_), direction(direction_),
        invDirection(1.0 / direction_.x, 1.0 / direction_.y, 1.0 / direction_.z)
  {
    assert(std::abs(direction.Length() - 1.0) < 1e-6);
  }

  const Vector& GetOrigin() const { return origin; }

  const Vector& GetDirection() const { return direction; }

  const Vector& GetInvDirection() const { return invDirection; }

  void Advance(double t) { origin = GetPoint(t); }
  Vector GetPoint(double t) const { return origin + direction * t; }

private:
  Vector origin;
  Vector direction;
  Vector invDirection;
};
