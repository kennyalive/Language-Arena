#pragma once

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
    void extend(TVector<T> point)
    {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        minPoint.z = std::min(minPoint.z, point.z);

        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
        maxPoint.z = std::max(maxPoint.z, point.z);
    }

    static TBoundingBox<T> boundsUnion(const TBoundingBox<T>& bounds,
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
