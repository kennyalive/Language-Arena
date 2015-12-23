#pragma once

#include "ray.h"
#include "vector.h"

struct Triangle
{
    Vector p[3];
};

struct RayTriangleIntersection
{
    double t;
    double b1;
    double b2;
    double rayEpsilon;
};

bool intersectTriangle(const Ray& ray, const Triangle& triangle, RayTriangleIntersection& intersection)
{
    Vector edge1 = triangle.p[1] - triangle.p[0];
    Vector edge2 = triangle.p[2] - triangle.p[0];

    Vector p = CrossProduct(ray.getDirection(), edge2);
    double divisor = DotProduct(edge1, p);

    if (divisor == 0.0) // todo: do we need to check against epsilon for better numeric stability?
        return false;

    const double invDivisor = 1.0 / divisor;

    // compute barycentric coordinate b1
    Vector t =  ray.getOrigin() - triangle.p[0];
    double b1 = invDivisor * DotProduct(t, p);
    if (b1 < 0.0 || b1 > 1.0)
        return false;

    // compute barycentric coordnate b2
    Vector q = CrossProduct(t, edge1);
    double b2 = invDivisor * DotProduct(ray.getDirection(), q);
    if (b2 < 0.0 || b1 + b2 > 1.0)
        return false;

    // compute distance from ray origin to intersection point
    double distance = invDivisor * DotProduct(edge2, q);
    if (distance < 0.0)
        return false;

    double rayEpsilon = 1e-3 * distance;
    intersection = { distance, b1, b2, rayEpsilon };
    return true;
}
