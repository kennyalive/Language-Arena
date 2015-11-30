#include "kdtree.h"
#include "random.h"
#include "vector.h"
#include "../../../scripts/common.h"

Vector uniformSampleSphere(double u1, double u2);

void benchmarkKdTree(const KdTree& kdtree)
{
    auto meshBounds = kdtree.getMeshBounds();

    // compute volume for generating test rays
    Vector diagonal = Vector(meshBounds.maxPoint - meshBounds.minPoint);
    double delta = 2.0 * diagonal.Length();

    auto bounds = BoundingBox(
        Vector(meshBounds.minPoint) - Vector(delta),
        Vector(meshBounds.maxPoint) + Vector(delta));

    Vector lastHit;
    double lastEpsilonOffset = 0.0;

    const auto benchmarkRays = 10000000;
    long raysTested = 0;

    Timer timer;

    while (raysTested < benchmarkRays)
    {
        // generate ray origin
        auto origin = Vector(
                             uniform(bounds.minPoint.x, bounds.maxPoint.x),
                             uniform(bounds.minPoint.y, bounds.maxPoint.y),
                             uniform(bounds.minPoint.z, bounds.maxPoint.z));

        if (uniform01() < 0.25)
            origin = lastHit;

        // generate ray direction;
        auto direction = uniformSampleSphere(uniform01(), uniform01());

        if (uniform01() < 1.0 / 32.0)
            direction.x = direction.y = 0.0;
        else if (uniform01() < 1.0 / 32.0)
            direction.x = direction.z = 0.0;
        else if (uniform01() < 1.0 / 32.0)
            direction.y = direction.z = 0.0;
        direction = direction.GetNormalized();

        // initialize ray
        auto ray = Ray(origin, direction);

        double epsilonOffset = 0.0;
        if (uniform01() < 0.25)
            epsilonOffset = lastEpsilonOffset;
        else if (uniform01() < 0.25)
            epsilonOffset = 1e-3;

        ray.advance(epsilonOffset);

        // test intersection
        Intersection intersection;
        bool hitFound = kdtree.intersect(ray, intersection);

        if (hitFound)
        {
            lastHit = ray.getPoint(intersection.rayT);
            lastEpsilonOffset = intersection.rayTEpsilon;
        }
        raysTested++;
    }

    const double benchmarkTime = timer.elapsed();
    auto speed = (benchmarkRays / 1000000.0) / benchmarkTime;
    printf("KdTree performance = %.2f MRays/sec\n", speed);
}
