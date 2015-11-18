import std.array;
import std.algorithm;
import std.conv;
import std.datetime;
import std.stdio;

import bounding_box;
import intersection;
import random;
import ray;
import sampling;
import vector;
import kdtree;
import triangle;

void benchmarkKdTree(ref const(KdTree) kdtree)
{
    // compute volume for generating test rays
    Vector diagonal = Vector(kdtree.meshBounds.maxPoint - kdtree.meshBounds.minPoint);
    double delta = 2.0 * diagonal.length();

    auto bounds = BoundingBox(
        Vector(kdtree.meshBounds.minPoint) - Vector(delta),
        Vector(kdtree.meshBounds.maxPoint) + Vector(delta));

    Vector lastHit;
    double lastEpsilonOffset = 0.0;

    immutable benchmarkRays = 10_000_000;
    long raysTested = 0;

    StopWatch sw;
    sw.start();

    while (raysTested < benchmarkRays)
    {
        // generate ray origin
        auto origin = Vector(
                             randForRange(bounds.minPoint.x, bounds.maxPoint.x),
                             randForRange(bounds.minPoint.y, bounds.maxPoint.y),
                             randForRange(bounds.minPoint.z, bounds.maxPoint.z));

        if (randDouble() < 0.25)
            origin = lastHit;

        // generate ray direction;
        auto direction = uniformSampleSphere(randDouble(), randDouble());

        if (randDouble() < 1.0 / 32.0)
            direction.x = direction.y = 0.0;
        else if (randDouble() < 1.0 / 32.0)
            direction.x = direction.z = 0.0;
        else if (randDouble() < 1.0 / 32.0)
            direction.y = direction.z = 0.0;
        direction = direction.getNormalized();

        // initialize ray
        auto ray = Ray(origin, direction);

        double epsilonOffset = 0.0;
        if (randDouble() < 0.25)
            epsilonOffset = lastEpsilonOffset;
        else if (randDouble() < 0.25)
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

    sw.stop();
    auto benchmarkTime = to!double(to!int(sw.peek().msecs)) / 1000.0;
    auto speed = (benchmarkRays / 1_000_000.0) / benchmarkTime;
    writefln("KdTree performance = %.2f MRays/sec ", speed);
}
