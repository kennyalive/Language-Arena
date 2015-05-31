import std.algorithm;
import std.typecons;

import ray;
import vector;

alias BoundingBox   = TBoundingBox!double;
alias BoundingBox_f = TBoundingBox!float;

struct TBoundingBox(T)
{
    pure: nothrow: @nogc:

    auto minPoint = TVector!T(T.infinity);
    auto maxPoint = TVector!T(-T.infinity);

    this (TVector!T minPoint, TVector!T maxPoint)
    {
        this.minPoint = minPoint;
        this.maxPoint = maxPoint;
    }

    this(TVector!T point)
    {
        minPoint = point;
        maxPoint = point;
    }

    void opOpAssign(string op)(TBoundingBox bounds)
    if (op == "+")
    {
        minPoint.x = min(minPoint.x, bounds.minPoint.x);
        minPoint.y = min(minPoint.y, bounds.minPoint.y);
        minPoint.z = min(minPoint.z, bounds.minPoint.z);

        maxPoint.x = max(maxPoint.x, bounds.maxPoint.x);
        maxPoint.y = max(maxPoint.y, bounds.maxPoint.y);
        maxPoint.z = max(maxPoint.z, bounds.maxPoint.z);
    }

    void extend(TVector!T point)
    {
        minPoint.x = min(minPoint.x, point.x);
        minPoint.y = min(minPoint.y, point.y);
        minPoint.z = min(minPoint.z, point.z);

        maxPoint.x = max(maxPoint.x, point.x);
        maxPoint.y = max(maxPoint.y, point.y);
        maxPoint.z = max(maxPoint.z, point.z);
    }

    alias Intersection = Tuple!(bool, "found", double, "t0", double, "t1");

    Intersection intersect(Ray ray) const
    {
        double t0 = 0.0;
        double t1 = double.infinity;

        foreach (i; 0..3)
        {
            double tNear = (minPoint[i] - ray.origin[i]) * ray.invDirection[i];
            double tFar =  (maxPoint[i] - ray.origin[i]) * ray.invDirection[i];

            if (tNear > tFar)
                swap(tNear, tFar);

            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar  < t1 ? tFar  : t1;
            if (t0 > t1)
                return Intersection(false, double.init, double.init);
        }
        return Intersection(true, t0, t1);
    }
}

TBoundingBox!T boundsUnion(T)(TBoundingBox!T bounds, TBoundingBox!T bounds2)
{
    return TBoundingBox!T
    (
        TVector!T
        (
            min(bounds.minPoint.x, bounds2.minPoint.x),
            min(bounds.minPoint.y, bounds2.minPoint.y),
            min(bounds.minPoint.z, bounds2.minPoint.z)
        ),
        TVector!T
        (
            max(bounds.maxPoint.x, bounds2.maxPoint.x),
            max(bounds.maxPoint.y, bounds2.maxPoint.y),
            max(bounds.maxPoint.z, bounds2.maxPoint.z)
        )
    );
}

version(unittest)
{
    import std.math;
    immutable auto relEps = 1e-4;
}

// ray intersection
unittest
{
    auto bounds = BoundingBox(Vector(-1), Vector(1));

    {
        auto ray = Ray(Vector(-2, 0, 0), Vector(1, 0, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 1.0, relEps));
        assert(approxEqual(intersection.t1, 3.0, relEps));
    }

    {
        auto ray = Ray(Vector(0, 0, 0), Vector(-1, 0, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 0.0, relEps));
        assert(approxEqual(intersection.t1, 1.0, relEps));
    }

    {
        auto ray = Ray(Vector(0, 1, 0), Vector(0, -1, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 0.0, relEps));
        assert(approxEqual(intersection.t1, 2.0, relEps));
    }

    {
        auto ray = Ray(Vector(0, 1, 0), Vector(0, 1, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 0.0, relEps));
        assert(approxEqual(intersection.t1, 0.0, relEps));
    }

    {
        auto ray = Ray(Vector(-1, 1, 0), Vector(0, 1, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 0.0, relEps));
        assert(approxEqual(intersection.t1, 0.0, relEps));
    }

    {
        auto ray = Ray(Vector(-1, 1, 0), Vector(0, -1, 0));
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, 0.0, relEps));
        assert(approxEqual(intersection.t1, 2.0, relEps));
    }

    {
        auto ray = Ray(Vector(-2, 0, 0), Vector(0, 1, 0));
        auto intersection = bounds.intersect(ray);
        assert(!intersection.found);
    }

    {
        auto ray = Ray(Vector(-3, 0, 0), Vector(1, 1, 0).getNormalized());
        auto intersection = bounds.intersect(ray);
        assert(!intersection.found);
    }

    {
        auto ray = Ray(Vector(-2, 0, 0), Vector(1, 1.0 + 1e-8, 0).getNormalized());
        auto intersection = bounds.intersect(ray);
        assert(!intersection.found);
    }

    {
        auto ray = Ray(Vector(-2, 0, 0), Vector(1, 1.0 - 1e-8, 0).getNormalized());
        auto intersection = bounds.intersect(ray);
        assert(intersection.found);
        assert(approxEqual(intersection.t0, sqrt(2.0), relEps));
        assert(approxEqual(intersection.t1, sqrt(2.0), relEps));
        assert(intersection.t0 < intersection.t1);
    }
}
