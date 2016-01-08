#pragma once

#include "ray.h"
#include "vector.h"
#include <algorithm>
#include <limits>

template <typename T>
struct TBoundingBox {
  TVector<T> minPoint;
  TVector<T> maxPoint;

  TBoundingBox()
      : minPoint(TVector<T>(std::numeric_limits<T>::infinity())),
        maxPoint(TVector<T>(-std::numeric_limits<T>::infinity()))
  {
  }

  TBoundingBox(TVector<T> minPoint, TVector<T> maxPoint)
      : minPoint(minPoint), maxPoint(maxPoint)
  {
  }

  TBoundingBox(TVector<T> point) : maxPoint(point), minPoint(point) {}

  void Extend(TVector<T> point)
  {
    minPoint.x = std::min(minPoint.x, point.x);
    minPoint.y = std::min(minPoint.y, point.y);
    minPoint.z = std::min(minPoint.z, point.z);

    maxPoint.x = std::max(maxPoint.x, point.x);
    maxPoint.y = std::max(maxPoint.y, point.y);
    maxPoint.z = std::max(maxPoint.z, point.z);
  }

  struct Intersection {
    bool found;
    double t0;
    double t1;
  };

  Intersection Intersect(const Ray& ray) const
  {
    double t0 = 0.0;
    double t1 = std::numeric_limits<double>::infinity();

    for (int i = 0; i < 3; i++) {
      double tNear =
          (minPoint[i] - ray.getOrigin()[i]) * ray.getInvDirection()[i];
      double tFar =
          (maxPoint[i] - ray.getOrigin()[i]) * ray.getInvDirection()[i];

      if (tNear > tFar)
        std::swap(tNear, tFar);

      t0 = tNear > t0 ? tNear : t0;
      t1 = tFar < t1 ? tFar : t1;
      if (t0 > t1)
        return {false, 0.0, 0.0};
    }
    return {true, t0, t1};
  }

  static TBoundingBox<T> Union(const TBoundingBox<T>& bounds,
                               const TBoundingBox<T>& bounds2)
  {
    return TBoundingBox<T>(
        TVector<T>(std::min(bounds.minPoint.x, bounds2.minPoint.x),
                   std::min(bounds.minPoint.y, bounds2.minPoint.y),
                   std::min(bounds.minPoint.z, bounds2.minPoint.z)),
        TVector<T>(std::max(bounds.maxPoint.x, bounds2.maxPoint.x),
                   std::max(bounds.maxPoint.y, bounds2.maxPoint.y),
                   std::max(bounds.maxPoint.z, bounds2.maxPoint.z)));
  }
};

using BoundingBox = TBoundingBox<double>;
using BoundingBox_f = TBoundingBox<float>;
