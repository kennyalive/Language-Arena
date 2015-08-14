#pragma once

#include "bounding_box.h"
#include "vector.h"

#include <array>
#include <cstdint>
#include <vector>

class TriangleMesh
{
public:
    TriangleMesh();

    TriangleMesh(const TriangleMesh&) = delete;
    TriangleMesh& operator=(const TriangleMesh&) = delete;

    struct Triangle
    {
        std::array<int32_t, 3> vertexIndices;
    };

    int32_t getTrianglesCount() const
    {
        return static_cast<int32_t>(triangles.size());
    }

    BoundingBox_f getTriangleBounds(int triangleIndex) const;
    BoundingBox_f getBounds() const;

public:
    std::vector<Vector_f> vertices;
    std::vector<Vector_f> normals;
    std::vector<Triangle> triangles;
};
