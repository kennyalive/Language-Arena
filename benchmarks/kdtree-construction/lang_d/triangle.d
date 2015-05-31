import std.typecons;
import ray;
import vector;

struct Triangle
{
    Vector[3] p;
}

alias TriangleIntersection = Tuple!(double, "t", double, "b1", double, "b2", double, "rayEpsilon");

pure nothrow @nogc
bool intersectTriangle(ref const(Ray) ray, ref const(Triangle) triangle, out TriangleIntersection intersection)
{
    Vector edge1 = triangle.p[1] - triangle.p[0];
    Vector edge2 = triangle.p[2] - triangle.p[0];

    Vector p = crossProduct(ray.direction, edge2);
    double divisor = dotProduct(edge1, p);

    if (divisor == 0.0) // todo: do we need to check against epsilon for better numeric stability?
        return false;

    double invDivisor = 1.0 / divisor;

    // compute barycentric coordinate b1
    Vector t =  ray.origin - triangle.p[0];
    double b1 = invDivisor * dotProduct(t, p);
    if (b1 < 0.0 || b1 > 1.0)
        return false;

    // compute barycentric coordnate b2
    Vector q = crossProduct(t, edge1);
    double b2 = invDivisor * dotProduct(ray.direction, q);
    if (b2 < 0.0 || b1 + b2 > 1.0)
        return false;

    // compute distance from ray origin to intersection point
    double distance = invDivisor * dotProduct(edge2, q);
    if (distance < 0.0)
        return false;

    double rayEpsilon = 1e-3 * distance;
    intersection = TriangleIntersection(distance, b1, b2, rayEpsilon);
    return true;
}

version(unittest)
{
    import std.math;
    immutable auto relEps = 1e-5;
}

// intersectTriangle
unittest
{
    {
        auto triangle = Triangle([Vector(-1.0, -1.0), Vector(1.0, -1.0), Vector(0.0, 1.0)]);
        auto ray = Ray(Vector(0, 0, 1), Vector(0, 0, -1));

        TriangleIntersection intersection;
        bool intersectionFound = intersectTriangle(ray, triangle, intersection);

        assert(intersectionFound);
        assert(approxEqual(intersection.t, 1.0, relEps));
        assert(approxEqual(intersection.b1, 0.25, relEps));
        assert(approxEqual(intersection.b2, 0.5, relEps));
    }

    {
        auto triangle = Triangle([Vector(-1.0, -1.0), Vector(1.0, -1.0), Vector(0.0, 1.0)]);
        auto ray = Ray(Vector(0, 0, 1), Vector(1, 0, 0));

        TriangleIntersection intersection;
        bool intersectionFound = intersectTriangle(ray, triangle, intersection);

        assert(!intersectionFound);
    }

    {
        auto triangle = Triangle([Vector(0.0, 0.0), Vector(1.0, 0.0), Vector(0.0, 1.0)]);
        auto ray = Ray(Vector(0, 0, 2), Vector(0, 0, -1));

        TriangleIntersection intersection;
        bool intersectionFound = intersectTriangle(ray, triangle, intersection);

        assert(intersectionFound);
        assert(approxEqual(intersection.t, 2.0, relEps));
        assert(approxEqual(intersection.b1, 0.0, relEps));
        assert(approxEqual(intersection.b2, 0.0, relEps));
    }

    {
        auto triangle = Triangle([Vector(0.0, 0.0), Vector(1.0, 0.0), Vector(0.0, 1.0)]);
        auto ray = Ray(Vector(0.5, 0, 2), Vector(0, 0, -1));

        TriangleIntersection intersection;
        bool intersectionFound = intersectTriangle(ray, triangle, intersection);

        assert(intersectionFound);
        assert(approxEqual(intersection.t, 2.0, relEps));
        assert(approxEqual(intersection.b1, 0.5, relEps));
        assert(approxEqual(intersection.b2, 0.0, relEps));
    }

    {
        auto triangle = Triangle([Vector(0.0, 0.0), Vector(1.0, 0.0), Vector(0.0, 1.0)]);
        auto ray = Ray(Vector(-1e-3, 0, 2), Vector(0, 0, -1));

        TriangleIntersection intersection;
        bool intersectionFound = intersectTriangle(ray, triangle, intersection);

        assert(!intersectionFound);
    }
}
