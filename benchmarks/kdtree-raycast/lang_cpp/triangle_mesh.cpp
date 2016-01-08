#include "bounding_box.h"
#include "triangle_mesh.h"

TriangleMesh::TriangleMesh() {}

int32_t TriangleMesh::GetTrianglesCount() const
{
  return static_cast<int32_t>(triangles.size());
}

BoundingBox_f TriangleMesh::GetTriangleBounds(int32_t triangleIndex) const
{
  const auto& p = triangles[triangleIndex].points;
  auto bounds = BoundingBox_f(vertices[p[0].vertexIndex]);
  bounds.Extend(vertices[p[1].vertexIndex]);
  bounds.Extend(vertices[p[2].vertexIndex]);
  return bounds;
}

BoundingBox_f TriangleMesh::GetBounds() const
{
  BoundingBox_f bounds;
  for (int32_t i = 0; i < GetTrianglesCount(); i++) {
    bounds = BoundingBox_f::Union(bounds, GetTriangleBounds(i));
  }
  return bounds;
}
