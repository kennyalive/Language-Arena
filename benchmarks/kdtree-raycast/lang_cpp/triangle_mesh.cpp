#include "triangle_mesh.h"
#include "bounding_box.h"

TriangleMesh::TriangleMesh()
{

}

BoundingBox_f TriangleMesh::getTriangleBounds(int triangleIndex) const
{
    auto indices = triangles[triangleIndex].vertexIndices;
    auto bounds = BoundingBox_f(vertices[indices[0]]);
    bounds.extend(vertices[indices[1]]);
    bounds.extend(vertices[indices[2]]);
    return bounds;
}

BoundingBox_f TriangleMesh::getBounds() const
{
    BoundingBox_f bounds;
    for (int32_t i = 0; i < getTrianglesCount(); ++i)
    {
        bounds = BoundingBox_f::boundsUnion(bounds, getTriangleBounds(i));
    }
    return bounds;
}
