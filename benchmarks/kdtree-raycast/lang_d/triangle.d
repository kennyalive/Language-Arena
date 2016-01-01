import ray;
import vector;

struct Triangle
{
    Vector[3] points;
}

struct TriangleIntersection
{
    double t = double.infinity;
    double epsilon = 0.0;
    double b1;
    double b2;
}

pure nothrow @nogc
bool intersectTriangle(ref const(Ray) ray, ref const(Triangle) triangle,
                       out TriangleIntersection intersection)
{
    Vector edge1 = triangle.points[1] - triangle.points[0];
    Vector edge2 = triangle.points[2] - triangle.points[0];

    Vector p = crossProduct(ray.direction, edge2);
    double divisor = dotProduct(edge1, p);

    // todo: do we need to check against epsilon for better numeric stability?
    if (divisor == 0.0)
        return false;

    double invDivisor = 1.0 / divisor;

    // compute barycentric coordinate b1
    Vector t =  ray.origin - triangle.points[0];
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
    intersection = TriangleIntersection(distance, rayEpsilon, b1, b2);
    return true;
}
