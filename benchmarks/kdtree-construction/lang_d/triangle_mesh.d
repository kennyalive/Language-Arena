import bounding_box;
import vector;

class TriangleMesh
{
    pure: nothrow: @nogc:

    struct TrianglePoint
    {
        int vertexIndex;
    }

    struct Triangle
    {
        TrianglePoint[3] points;
    }

    int getTrianglesCount() const
    {
        return cast(int)triangles.length;
    }

    BoundingBox_f getTriangleBounds(int triangleIndex) const 
    {
        auto p = triangles[triangleIndex].points;
        auto bounds = BoundingBox_f(vertices[p[0].vertexIndex]);
        bounds.extend(vertices[p[1].vertexIndex]);
        bounds.extend(vertices[p[2].vertexIndex]);
        return bounds;
    }

    BoundingBox_f getBounds() const
    {
        BoundingBox_f bounds;
        foreach (i; 0..getTrianglesCount())
            bounds = boundsUnion(bounds, getTriangleBounds(i));
        return bounds;
    }

public:
    Vector_f[] vertices;
    Vector_f[] normals;
    Triangle[] triangles;
}
