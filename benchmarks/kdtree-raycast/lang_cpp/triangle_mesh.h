#pragma once

#include "bounding_box.h"
#include "vector.h"
#include <array>
#include <cstdint>
#include <vector>

class TriangleMesh {
public:
  struct TrianglePoint {
    int32_t vertexIndex;
  };

  struct Triangle {
    std::array<TrianglePoint, 3> points;
  };

public:
  TriangleMesh();

  int32_t GetTrianglesCount() const;
  BoundingBox_f GetTriangleBounds(int32_t triangleIndex) const;
  BoundingBox_f GetBounds() const;

public:
  std::vector<Vector_f> vertices;
  std::vector<Vector_f> normals;
  std::vector<Triangle> triangles;
};
