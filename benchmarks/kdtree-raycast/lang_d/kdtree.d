import std.exception;
import std.stdio;
import common;
import bounding_box;
import ray;
import triangle;
import triangle_mesh;
import vector;

class KdTree
{
    pure nothrow @nogc
    this(immutable(Node)[] nodes, immutable(int)[] triangleIndices, 
         immutable(TriangleMesh) mesh)
    {
        this.nodes = nodes;
        this.triangleIndices = triangleIndices;
        this.mesh = mesh;
        this.meshBounds = mesh.getBounds();
    }

    this(string fileName, immutable(TriangleMesh) mesh) immutable
    {
        try
        {
            this.mesh = mesh;
            this.meshBounds = mesh.getBounds();

            auto file = File(fileName, "rb");

            int[1] nodesCount;
            file.rawRead(nodesCount);
            nodes.length = nodesCount[0];
            file.rawRead(cast(Node[])nodes);

            int[1] triangleIndicesCount;
            file.rawRead(triangleIndicesCount);
            triangleIndices.length = triangleIndicesCount[0];
            file.rawRead(cast(int[])triangleIndices);
        }
        catch (ErrnoException)
        {
            runtimeError("failed to read kdtree file: " ~ fileName);
        }
    }

    void saveToFile(string fileName)
    {
        try
        {
            auto file = File(fileName, "wb");

            int[1] nodesCount = cast(int) nodes.length;
            file.rawWrite(nodesCount);
            file.rawWrite(nodes);

            int[1] triangleIndicesCount = cast(int) triangleIndices.length;
            file.rawWrite(triangleIndicesCount);
            file.rawWrite(triangleIndices);
        }
        catch (ErrnoException)
        {
            runtimeError("failed to write kdtree file: " ~ fileName);
        }
    }

    struct Intersection
    {
        double t = double.infinity;
        double epsilon = 0.0;
    }

    pure nothrow @nogc
    bool intersect(Ray ray, out Intersection intersection) const
    {
        auto boundsIntersection = meshBounds.intersect(ray);
        if (!boundsIntersection.found)
            return false;

        double tMin = boundsIntersection.t0;
        double tMax = boundsIntersection.t1;

        auto node = &nodes[0];

        struct TraversalInfo
        {
            immutable(Node)* node;
            double tMin;
            double tMax;
        }
        TraversalInfo[traversalMaxDepth] traversalStack = void;
        int traversalStackSize = 0;

        Triangle.Intersection closestIntersection;
        while (closestIntersection.t > tMin)
        {
            if (node.isInteriorNode())
            {
                int axis = node.getInteriorNodeSplitAxis();
                double distanceToSplitPlane = node.split - ray.origin[axis];
                auto belowChild = node + 1;
                auto aboveChild = &nodes[node.getInteriorNodeAboveChild()];

                if (distanceToSplitPlane != 0.0) // general case
                {
                    immutable(Node)* firstChild, secondChild;
                    if (distanceToSplitPlane > 0.0)
                    {
                        firstChild = belowChild;
                        secondChild = aboveChild;
                    }
                    else
                    {
                        firstChild = aboveChild;
                        secondChild = belowChild;
                    }

                    // tSplit != 0 (since distanceToSplitPlane != 0)
                    double tSplit = distanceToSplitPlane * ray.invDirection[axis];

                    if (tSplit >= tMax || tSplit < 0.0)
                    {
                        node = firstChild;
                    }
                    else if (tSplit <= tMin)
                    {
                        node = secondChild;
                    }
                    else // tMin < tSplit < tMax
                    {
                        assert(traversalStackSize < traversalMaxDepth);
                        traversalStack[traversalStackSize++] =
                            TraversalInfo(secondChild, tSplit, tMax);
                        node = firstChild;
                        tMax = tSplit;
                    }
                }
                else // special case, distanceToSplitPlane == 0.0
                {
                    if (ray.direction[axis] > 0.0)
                    {
                        if (tMin > 0.0)
                            node = aboveChild;
                        else // tMin == 0.0
                        {
                            assert(traversalStackSize < traversalMaxDepth);
                            traversalStack[traversalStackSize++] =
                                TraversalInfo(aboveChild, 0.0, tMax);
                            // check single point [0.0, 0.0]
                            node = belowChild;
                            tMax = 0.0;
                        }
                    }
                    else if (ray.direction[axis] < 0.0)
                    {
                        if (tMin > 0.0)
                            node = belowChild;
                        else // tMin == 0.0
                        {
                            assert(traversalStackSize < traversalMaxDepth);
                            traversalStack[traversalStackSize++] =
                                TraversalInfo(belowChild, 0.0, tMax);
                            // check single point [0.0, 0.0]
                            node = aboveChild;
                            tMax = 0.0;
                        }
                    }
                    else // ray.direction[axis] == 0.0
                    {
                        // for both nodes check [tMin, tMax] range
                        assert(traversalStackSize < traversalMaxDepth);
                        traversalStack[traversalStackSize++] =
                            TraversalInfo(aboveChild, tMin, tMax);
                        node = belowChild;
                    }
                }
            }
            else // leaf node
            {
                intersectLeafTriangles(*node, ray, closestIntersection);

                if (traversalStackSize == 0)
                    break;

                --traversalStackSize;
                node = traversalStack[traversalStackSize].node;
                tMin = traversalStack[traversalStackSize].tMin;
                tMax = traversalStack[traversalStackSize].tMax;
            }
        } // while (closestIntersection.t > tMin)

        if (closestIntersection.t == double.infinity)
            return false;

        intersection.t = closestIntersection.t;
        intersection.epsilon = closestIntersection.epsilon;
        return true;
    }

    pure nothrow @nogc
    private void intersectLeafTriangles(Node leaf, Ray ray,
                    ref Triangle.Intersection closestIntersection) const
    {
        int trianglesCount = leaf.getLeafTrianglesCount();
        if (trianglesCount == 1)
        {
            auto p = mesh.triangles[leaf.index].points;
            Triangle triangle = {[
                Vector(mesh.vertices[p[0].vertexIndex]),
                Vector(mesh.vertices[p[1].vertexIndex]),
                Vector(mesh.vertices[p[2].vertexIndex])
            ]};
            Triangle.Intersection intersection;
            bool hitFound = intersectTriangle(ray, triangle, intersection);
            if (hitFound && intersection.t < closestIntersection.t)
                closestIntersection = intersection;
        }
        else
        {
            foreach (i; 0..trianglesCount)
            {
                auto p = mesh.triangles[triangleIndices[leaf.index + i]].points;
                Triangle triangle = {[
                    Vector(mesh.vertices[p[0].vertexIndex]),
                    Vector(mesh.vertices[p[1].vertexIndex]),
                    Vector(mesh.vertices[p[2].vertexIndex])
                ]};
                Triangle.Intersection intersection;
                bool hitFound = intersectTriangle(ray,  triangle, intersection);
                if (hitFound && intersection.t < closestIntersection.t)
                    closestIntersection = intersection;
            }
        }
    }

private:
    struct Node
    {   
        pure: nothrow: @nogc:

        // Determines node type (interior/leaf).
        // Additionally: 
        //  for interior node specifies split axis and above child index
        //  for leaf node specifies the number of triangles that intersect it
        uint header;

        union
        {
            float split; // used in interior node to determine split position
            int index; // used in leaf node to locate leaf triangle(s)
        }

        enum int maxNodesCount = 0x4000_0000; // max ~ 1 billion nodes
        enum uint leafNodeFlags = 3;

        void initInteriorNode(int axis, int aboveChild, float split)
        {
            // 0 - x axis, 1 - y axis, 2 - z axis
            assert(axis >= 0 && axis < 3);
            assert(aboveChild < maxNodesCount);

            header = axis | (aboveChild << 2);
            this.split = split;
        }

        void initEmptyLeaf()
        {
            header = leafNodeFlags; // = 3
            index = 0; // not used for empty leaf, just set default value
        }

        void initLeafWithSingleTriangle(int triangleIndex)
        {
            header = leafNodeFlags | (1 << 2); // = 7
            index = triangleIndex;
        }

        void initLeafWithMultipleTriangles(int numTriangles,
            int triangleIndicesOffset)
        {
            assert(numTriangles > 1);
            // header == 11, 15, 19, ... (for numTriangles = 2, 3, 4, ...)
            header = leafNodeFlags | (numTriangles << 2);
            index = triangleIndicesOffset;
        }

        bool isLeaf() const
        {
            return (header & leafNodeFlags) == leafNodeFlags;
        }

        bool isInteriorNode() const
        {
            return !isLeaf;
        }

        int getLeafTrianglesCount() const
        {
            assert(isLeaf);
            return header >> 2;
        }

        int getInteriorNodeSplitAxis() const
        {
            assert(isInteriorNode);
            return header & leafNodeFlags;
        }

        int getInteriorNodeAboveChild() const
        {
            assert(isInteriorNode);
            return header >> 2;
        }
    } // KdNode

private:
    enum traversalMaxDepth = 64;
    immutable(Node)[] nodes;
    immutable(int)[] triangleIndices;

public:
    immutable(TriangleMesh) mesh;
    immutable(BoundingBox_f) meshBounds;
}
