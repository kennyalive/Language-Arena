import std.conv;
import std.datetime;
import std.math;
import std.stdio;
import bounding_box;
import common;
import kdtree;
import random;
import ray;
import triangle;
import vector;

immutable benchmarkRaysCount = 10_000_000;

private Vector uniformSampleSphere(double u1, double u2)
{
    assert(u1 >= 0.0 && u1 < 1.0);
    assert(u2 >= 0.0 && u2 < 1.0);

    double z = 1.0 - 2.0 * u1;
    double r = sqrt(1.0 - z*z);
    double phi = 2.0 * PI * u2;
    double x = r * cos(phi);
    double y = r * sin(phi);
    return Vector(x, y, z);
}

private struct RayGenerator
{
    this(BoundingBox_f meshBounds)
    {
        auto diagonal = Vector(meshBounds.maxPoint - meshBounds.minPoint);
        double delta = 2.0 * diagonal.length();

        raysBounds = BoundingBox(
            Vector(meshBounds.minPoint) - Vector(delta),
            Vector(meshBounds.maxPoint) + Vector(delta));
    }

    Ray generateRay(Vector lastHit, double lastHitEpsilon) const
    {
        // generate ray origin
        Vector origin;
        origin.x = randForRange(raysBounds.minPoint.x, raysBounds.maxPoint.x);
        origin.y = randForRange(raysBounds.minPoint.y, raysBounds.maxPoint.y);
        origin.z = randForRange(raysBounds.minPoint.z, raysBounds.maxPoint.z);

        const bool useLastHit = randDouble() < 0.25;
        if (useLastHit)
            origin = lastHit;

        // generate ray direction;  
        auto u1 = randDouble();
        auto u2 = randDouble();
        auto direction = uniformSampleSphere(u1, u2);

        if (randDouble() < 1.0 / 32.0 && direction.z != 0.0)
            direction.x = direction.y = 0.0;
        else if (randDouble() < 1.0 / 32.0 && direction.y != 0.0)
            direction.x = direction.z = 0.0;
        else if (randDouble() < 1.0 / 32.0 && direction.x != 0.0)
            direction.y = direction.z = 0.0;
        direction = direction.getNormalized();

        // initialize ray
        auto ray = Ray(origin, direction);

        if (useLastHit)
        {
            if (randDouble() < 0.25)
                ray.advance(lastHitEpsilon);
            else if (randDouble() < 0.25)
                ray.advance(1e-3);
        }
        return ray;
    }

    private BoundingBox raysBounds;
}

int benchmarkKdTree(const(KdTree) kdtree)
{
    StopWatch sw;
    sw.start();

    Vector lastHit;
    double lastHitEpsilon = 0.0;
    auto rayGenerator = RayGenerator(kdtree.meshBounds);

    for (int raysTested = 0; raysTested < benchmarkRaysCount; raysTested++)
    {
        const Ray ray = rayGenerator.generateRay(lastHit, lastHitEpsilon);

        RayIntersection intersection;
        bool hitFound = kdtree.intersect(ray, intersection);

        if (hitFound)
        {
            lastHit = ray.getPoint(intersection.t);
            lastHitEpsilon = intersection.epsilon;
        }
    }

    sw.stop();
    return to!int(sw.peek().msecs());
}

void validateKdTree(const(KdTree) kdtree, int raysCount)
{
    Vector lastHit;
    double lastHitEpsilon = 0.0;
    auto rayGenerator = RayGenerator(kdtree.meshBounds);

    for (int raysTested = 0; raysTested < raysCount; raysTested++)
    {
        const Ray ray = rayGenerator.generateRay(lastHit, lastHitEpsilon);

        RayIntersection kdTreeIntersection;
        bool kdTreeHitFound = kdtree.intersect(ray, kdTreeIntersection);

        RayIntersection bruteForceIntersection;
        bool bruteForceHitFound = false;

        foreach (i; 0..kdtree.mesh.getTrianglesCount())
        {
            auto p = kdtree.mesh.triangles[i].points;

            Triangle triangle = {[
                Vector(kdtree.mesh.vertices[p[0].vertexIndex]),
                Vector(kdtree.mesh.vertices[p[1].vertexIndex]),
                Vector(kdtree.mesh.vertices[p[2].vertexIndex])
            ]};

            TriangleIntersection intersection;
            bool hitFound = intersectTriangle(ray, triangle, intersection);

            if (hitFound && intersection.t < bruteForceIntersection.t)
            {
                bruteForceIntersection.t = intersection.t;
                bruteForceHitFound = true;
            }
        } 

        if (kdTreeHitFound != bruteForceHitFound ||
            kdTreeIntersection.t != bruteForceIntersection.t)
        {
            writefln("KdTree accelerator test failure:\n"
                     "kdtree hit: %s\n"
                     "actual hit: %s\n"
                     "kdtree T %.16g [%a]\n"
                     "actual T %.16g [%a]\n"
                     "ray origin: (%a, %a, %a)\n"
                     "ray direction: (%a, %a, %a)",
                     kdTreeHitFound ? "true":"false",
                     bruteForceHitFound ? "true":"false",
                     kdTreeIntersection.t, kdTreeIntersection.t,
                     bruteForceIntersection.t, bruteForceIntersection.t,
                     ray.origin.x, ray.origin.y, ray.origin.z,
                     ray.direction.x, ray.direction.y, ray.direction.z);
            validationError("kdtreee traversal error detected");
        }

        if (bruteForceHitFound)
        {
            lastHit = ray.getPoint(bruteForceIntersection.t);
            lastHitEpsilon = bruteForceIntersection.epsilon;
        }
    }
}
