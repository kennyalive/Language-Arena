#include "benchmark.h"
#include "common.h"
#include "kdtree.h"
#include "random.h"
#include "triangle.h"
#include "vector.h"
#include <cassert>

namespace {
const double PI = 3.14159265358979323846;

Vector UniformSampleSphere()
{
  auto u1 = RandDouble();
  auto u2 = RandDouble();
  assert(u1 >= 0.0 && u1 < 1.0);
  assert(u2 >= 0.0 && u2 < 1.0);

  double z = 1.0 - 2.0 * u1;
  double r = sqrt(1.0 - z * z);
  double phi = 2.0 * PI * u2;
  double x = r * cos(phi);
  double y = r * sin(phi);
  return Vector(x, y, z);
}

class RayGenerator {
public:
  RayGenerator(const BoundingBox& meshBounds)
  {
    auto diagonal = meshBounds.maxPoint - meshBounds.minPoint;
    double delta = 2.0 * diagonal.Length();

    raysBounds = BoundingBox(meshBounds.minPoint - Vector(delta),
                             meshBounds.maxPoint + Vector(delta));
  }

  Ray GenerateRay(const Vector& lastHit, double lastHitEpsilon) const
  {
    // generate ray origin
    Vector origin;
    origin.x = RandFromRange(raysBounds.minPoint.x, raysBounds.maxPoint.x);
    origin.y = RandFromRange(raysBounds.minPoint.y, raysBounds.maxPoint.y);
    origin.z = RandFromRange(raysBounds.minPoint.z, raysBounds.maxPoint.z);

    const bool useLastHit = RandDouble() < 0.25;
    if (useLastHit)
      origin = lastHit;

    // generate ray direction;
    auto direction = UniformSampleSphere();

    if (RandDouble() < 1.0 / 32.0 && direction.z != 0.0)
      direction.x = direction.y = 0.0;
    else if (RandDouble() < 1.0 / 32.0 && direction.y != 0.0)
      direction.x = direction.z = 0.0;
    else if (RandDouble() < 1.0 / 32.0 && direction.x != 0.0)
      direction.y = direction.z = 0.0;
    direction = direction.GetNormalized();

    // initialize ray
    auto ray = Ray(origin, direction);

    if (useLastHit) {
      ray.Advance(lastHitEpsilon);
    }
    else {
      ray.Advance(1e-3);
    }
    return ray;
  }

private:
  BoundingBox raysBounds;
};
} // namespace

int BenchmarkKdTree(const KdTree& kdTree)
{
  Timer timer;

  Vector lastHit =
    (kdTree.GetMeshBounds().minPoint + kdTree.GetMeshBounds().maxPoint) * 0.5;
  double lastHitEpsilon = 0.0;
  auto rayGenerator = RayGenerator(kdTree.GetMeshBounds());

  for (int raysTested = 0; raysTested < benchmarkRaysCount; raysTested++) {
    const Ray ray = rayGenerator.GenerateRay(lastHit, lastHitEpsilon);

    KdTree::Intersection intersection;
    bool hitFound = kdTree.Intersect(ray, intersection);

    if (hitFound) {
      lastHit = ray.GetPoint(intersection.t);
      lastHitEpsilon = intersection.epsilon;
    }

    //// debug output
    //if (raysTested < 1024) {
    //  if (hitFound)
    //    printf("%d: found: %s, lastHit: %.14f %.14f %.14f\n", raysTested,
    //           hitFound ? "true" : "false", lastHit.x, lastHit.y, lastHit.z);
    //  else
    //    printf("%d: found: %s\n", raysTested, hitFound ? "true" : "false");
    //}
  }
  return timer.ElapsedMilliseconds();
}

void ValidateKdTree(const KdTree& kdTree, int raysCount)
{
  Vector lastHit =
      (kdTree.GetMeshBounds().minPoint + kdTree.GetMeshBounds().maxPoint) * 0.5;
  double lastHitEpsilon = 0.0;
  auto rayGenerator = RayGenerator(kdTree.GetMeshBounds());

  for (int raysTested = 0; raysTested < raysCount; raysTested++) {
    const Ray ray = rayGenerator.GenerateRay(lastHit, lastHitEpsilon);

    KdTree::Intersection kdTreeIntersection;
    bool kdTreeHitFound = kdTree.Intersect(ray, kdTreeIntersection);

    KdTree::Intersection bruteForceIntersection;
    bool bruteForceHitFound = false;

    for (int32_t i = 0; i < kdTree.GetMesh().GetTrianglesCount(); i++) {
      const auto& p = kdTree.GetMesh().triangles[i].points;

      Triangle triangle = {
          {Vector(kdTree.GetMesh().vertices[p[0].vertexIndex]),
           Vector(kdTree.GetMesh().vertices[p[1].vertexIndex]),
           Vector(kdTree.GetMesh().vertices[p[2].vertexIndex])}};

      Triangle::Intersection intersection;
      bool hitFound = IntersectTriangle(ray, triangle, intersection);

      if (hitFound && intersection.t < bruteForceIntersection.t) {
        bruteForceIntersection.t = intersection.t;
        bruteForceHitFound = true;
      }
    }

    if (kdTreeHitFound != bruteForceHitFound ||
        kdTreeIntersection.t != bruteForceIntersection.t) {
      const auto& o = ray.GetOrigin();
      const auto& d = ray.GetDirection();
      printf("KdTree accelerator test failure:\n"
             "KdTree hit: %s\n"
             "actual hit: %s\n"
             "KdTree T %.16g [%a]\n"
             "actual T %.16g [%a]\n"
             "ray origin: (%a, %a, %a)\n"
             "ray direction: (%a, %a, %a)\n",
             kdTreeHitFound ? "true" : "false",
             bruteForceHitFound ? "true" : "false", kdTreeIntersection.t,
             kdTreeIntersection.t, bruteForceIntersection.t,
             bruteForceIntersection.t, o.x, o.y, o.z, d.x, d.y, d.z);
      ValidationError("KdTree traversal error detected");
    }

    if (bruteForceHitFound) {
      lastHit = ray.GetPoint(bruteForceIntersection.t);
      lastHitEpsilon = bruteForceIntersection.epsilon;
    }
  }
}
