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

private Vector uniformSampleSphere()
{
    auto u1 = randDouble();
    auto u2 = randDouble();
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
    this(BoundingBox meshBounds)
    {
        auto diagonal = meshBounds.maxPoint - meshBounds.minPoint;
        double delta = 2.0 * diagonal.length();

        raysBounds = BoundingBox(
            meshBounds.minPoint - Vector(delta),
            meshBounds.maxPoint + Vector(delta));
    }

    Ray generateRay(Vector lastHit, double lastHitEpsilon) const
    {
        // generate ray origin
        Vector origin;
        origin.x = randFromRange(raysBounds.minPoint.x, raysBounds.maxPoint.x);
        origin.y = randFromRange(raysBounds.minPoint.y, raysBounds.maxPoint.y);
        origin.z = randFromRange(raysBounds.minPoint.z, raysBounds.maxPoint.z);

        const bool useLastHit = randDouble() < 0.25;
        if (useLastHit)
            origin = lastHit;

        // generate ray direction;  
        auto direction = uniformSampleSphere();

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
            ray.advance(lastHitEpsilon);
        else
            ray.advance(1e-3);
        return ray;
    }

    private BoundingBox raysBounds;
}

int benchmarkKdTree(const(KdTree) kdTree)
{
    StopWatch sw;
    sw.start();

    Vector lastHit = (kdTree.meshBounds.minPoint + kdTree.meshBounds.maxPoint) * 0.5;
    double lastHitEpsilon = 0.0;
    auto rayGenerator = RayGenerator(kdTree.meshBounds);

    for (int raysTested = 0; raysTested < benchmarkRaysCount; raysTested++)
    {
        const Ray ray = rayGenerator.generateRay(lastHit, lastHitEpsilon);

        KdTree.Intersection intersection;
        bool hitFound = kdTree.intersect(ray, intersection);

        if (hitFound)
        {
            lastHit = ray.getPoint(intersection.t);
            lastHitEpsilon = intersection.epsilon;
        }

        //// debug output
        //if (raysTested < 1024)
        //{
        //  if (hitFound)
        //    writefln("%d: found: %s, lastHit: %.14f %.14f %.14f",
        //      raysTested, hitFound ? "true" : "false", lastHit.x, lastHit.y, lastHit.z);
        //  else
        //    writefln("%d: found: %s", raysTested, hitFound ? "true" : "false");
        //}
    }
    sw.stop();
    return to!int(sw.peek().msecs());
}

void validateKdTree(const(KdTree) kdTree, int raysCount)
{
    Vector lastHit = (kdTree.meshBounds.minPoint + kdTree.meshBounds.maxPoint) * 0.5;
    double lastHitEpsilon = 0.0;
    auto rayGenerator = RayGenerator(kdTree.meshBounds);

    for (int raysTested = 0; raysTested < raysCount; raysTested++)
    {
        const Ray ray = rayGenerator.generateRay(lastHit, lastHitEpsilon);

        KdTree.Intersection kdTreeIntersection;
        bool kdTreeHitFound = kdTree.intersect(ray, kdTreeIntersection);

        KdTree.Intersection bruteForceIntersection;
        bool bruteForceHitFound = false;

        foreach (i; 0..kdTree.mesh.getTrianglesCount())
        {
            auto p = kdTree.mesh.triangles[i].points;

            Triangle triangle = {[
                Vector(kdTree.mesh.vertices[p[0].vertexIndex]),
                Vector(kdTree.mesh.vertices[p[1].vertexIndex]),
                Vector(kdTree.mesh.vertices[p[2].vertexIndex])
            ]};

            Triangle.Intersection intersection;
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
            const auto o = ray.origin;
            const auto d = ray.direction;
            writefln("KdTree accelerator test failure:\n" ~
                     "KdTree hit: %s\n" ~
                     "actual hit: %s\n" ~
                     "KdTree T %.16g [%a]\n" ~
                     "actual T %.16g [%a]\n" ~
                     "ray origin: (%a, %a, %a)\n" ~
                     "ray direction: (%a, %a, %a)",
                     kdTreeHitFound ? "true":"false",
                     bruteForceHitFound ? "true":"false",
                     kdTreeIntersection.t, kdTreeIntersection.t,
                     bruteForceIntersection.t, bruteForceIntersection.t,
                     o.x, o.y, o.z, d.x, d.y, d.z);
            validationError("KdTreee traversal error detected");
        }

        if (bruteForceHitFound)
        {
            lastHit = ray.getPoint(bruteForceIntersection.t);
            lastHitEpsilon = bruteForceIntersection.epsilon;
        }
    }
}
