import bounding_box;
import vector;

class TriangleMesh
{
    pure: nothrow: @nogc:

    struct Triangle
    {
        int[3] vertex_indices;
    }

    int getTrianglesCount() const
    {
        return cast(int)triangles.length;
    }

    BoundingBox_f getTriangleBounds(int triangle_index) const 
    {
        int[3] indices = triangles[triangle_index].vertex_indices;
        auto bounds = BoundingBox_f(vertices[indices[0]]);
        bounds.extend(vertices[indices[1]]);
        bounds.extend(vertices[indices[2]]);
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
