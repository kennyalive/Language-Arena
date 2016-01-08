#include "triangle.h"

bool IntersectTriangle(const Ray& ray, const Triangle& triangle,
  Triangle::Intersection& intersection)
{
  Vector edge1 = triangle.points[1] - triangle.points[0];
  Vector edge2 = triangle.points[2] - triangle.points[0];

  Vector p = CrossProduct(ray.GetDirection(), edge2);
  double divisor = DotProduct(edge1, p);

  // todo: do we need to check against epsilon for better numeric stability?
  if (divisor == 0.0)
    return false;

  const double invDivisor = 1.0 / divisor;

  // compute barycentric coordinate b1
  Vector t = ray.GetOrigin() - triangle.points[0];
  double b1 = invDivisor * DotProduct(t, p);
  if (b1 < 0.0 || b1 > 1.0)
    return false;

  // compute barycentric coordnate b2
  Vector q = CrossProduct(t, edge1);
  double b2 = invDivisor * DotProduct(ray.GetDirection(), q);
  if (b2 < 0.0 || b1 + b2 > 1.0)
    return false;

  // compute distance from ray origin to intersection point
  double distance = invDivisor * DotProduct(edge2, q);
  if (distance < 0.0)
    return false;

  intersection.t = distance;
  intersection.epsilon = 1e-3 * distance;
  intersection.b1 = b1;
  intersection.b2 = b2;
  return true;
}