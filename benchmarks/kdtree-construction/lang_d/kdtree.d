import std.stdio;
import bounding_box;
import intersection;
import ray;
import triangle_mesh;
import vector;
import triangle;

struct KdTree
{
    pure
    this(immutable(Node)[] nodes, immutable(int)[] triangleIndices, immutable(TriangleMesh) mesh)
    {
        this.nodes = nodes;
        this.triangleIndices = triangleIndices;
        this.mesh = mesh;
        this.meshBounds = mesh.getBounds();
    }

    this(string kdtreeFileName, immutable(TriangleMesh) mesh)
    {
        this.mesh = mesh;
        this.meshBounds = mesh.getBounds();

        auto file = File(kdtreeFileName, "rb");

        int[1] nodesCount;
        file.rawRead(nodesCount);

        nodes.length = nodesCount[0];
        file.rawRead(cast(Node[])nodes);

        int[1] triangleIndicesCount;
        file.rawRead(triangleIndicesCount);

        triangleIndices.length = triangleIndicesCount[0];
        file.rawRead(cast(int[])triangleIndices);
    }

    pure nothrow @nogc
    bool intersect(Ray ray, out Intersection intersection) const
    {
        auto boundsIntersection = meshBounds.intersect(ray);
        if (!boundsIntersection.found)
            return false;

        struct TraversalInfo
        {
            immutable(Node)* node;
            double tMin;
            double tMax;
        }
        TraversalInfo[traversalMaxDepth] traversalStack = void;
        int traversalStackSize = 0;

        double tMin = boundsIntersection.t0;
        double tMax = boundsIntersection.t1;

        auto node = &nodes[0];

        TriangleIntersection closestIntersection;
        closestIntersection.t = double.infinity;

        while (closestIntersection.t > tMin)
        {
            if (node.isInteriorNode())
            {
                int axis = node.getInteriorNodeSplitAxis();
                double distanceToSplitPlane = node.split - ray.origin[axis];

                if (distanceToSplitPlane == 0.0)
                {
                    if (ray.direction[axis] > 0.0)
                        node = &nodes[node.getInteriorNodeAboveChild];
                    else
                        node += 1;
                    // todo: check case when ray.direction[axis] == 0.0, probably in this case we 
                    // we need to process both nodes to avoid rendering artifacts (although rare cases)
                }
                else
                {
                    immutable(Node)* firstChild, secondChild;
                    if (distanceToSplitPlane > 0.0)
                    {
                        firstChild = node + 1;
                        secondChild = &nodes[node.getInteriorNodeAboveChild()];
                    }
                    else
                    {
                        firstChild = &nodes[node.getInteriorNodeAboveChild()];
                        secondChild = node + 1;
                    }

                    double tSplit = distanceToSplitPlane * ray.invDirection[axis]; // != 0
                    if (tSplit >= tMax || tSplit < 0.0)
                        node = firstChild;
                    else if (tSplit <= tMin)
                        node = secondChild;
                    else // tMin < tSplit < tMax
                    {
                        assert(traversalStackSize < traversalMaxDepth);
                        traversalStack[traversalStackSize++] = TraversalInfo(secondChild, tSplit, tMax);
                        node = firstChild;
                        tMax = tSplit;
                    }
                }
            }
            else // leaf node
            {
                int trianglesCount = node.getLeafTrianglesCount();
                if (trianglesCount == 1)
                {
                    auto indices = mesh.triangles[node.index].vertex_indices;
                    Triangle triangle;
                    triangle.p[0] = Vector(mesh.vertices[indices[0]]);
                    triangle.p[1] = Vector(mesh.vertices[indices[1]]);
                    triangle.p[2] = Vector(mesh.vertices[indices[2]]);

                    TriangleIntersection triangleIntersection;
                    bool intersectionFound = intersectTriangle(ray,  triangle, triangleIntersection);
                    if (intersectionFound && triangleIntersection.t < closestIntersection.t)
                    {
                        closestIntersection = triangleIntersection;
                    }
                }
                else
                {
                    foreach (i; 0..trianglesCount)
                    {
                        auto indices = mesh.triangles[triangleIndices[node.index + i]].vertex_indices;
                        Triangle triangle;
                        triangle.p[0] = Vector(mesh.vertices[indices[0]]);
                        triangle.p[1] = Vector(mesh.vertices[indices[1]]);
                        triangle.p[2] = Vector(mesh.vertices[indices[2]]);

                        TriangleIntersection triangleIntersection;
                        bool intersectionFound = intersectTriangle(ray,  triangle, triangleIntersection);
                        if (intersectionFound && triangleIntersection.t < closestIntersection.t)
                        {
                            closestIntersection = triangleIntersection;
                        }
                    }
                }

                if (traversalStackSize == 0)
                    break;

                --traversalStackSize;
                node = traversalStack[traversalStackSize].node;
                tMin = traversalStack[traversalStackSize].tMin;
                tMax = traversalStack[traversalStackSize].tMax;
            }
        }

        bool isHit = closestIntersection.t < double.infinity;
        if (isHit)
        {
            intersection.rayT = closestIntersection.t;
            intersection.rayTEpsilon = closestIntersection.rayEpsilon;
        }
        return isHit;
    }

    void saveToFile(string fileName)
    {
        auto file = File(fileName, "wb");

        int[1] nodesCount = cast(int) nodes.length;
        file.rawWrite(nodesCount);
        file.rawWrite(nodes);

        int[1] triangleIndicesCount = cast(int) triangleIndices.length;
        file.rawWrite(triangleIndicesCount);
        file.rawWrite(triangleIndices);
    }

    ulong getHash() const
    {
        uint hash1 = 0;
        foreach (node; nodes)
            hash1 = hash1 * 33 + node.header + node.index;

        uint hash2 = 0;
        foreach (index; triangleIndices)
            hash2 = hash2 * 33 + index;

        ulong hash = hash1 + ((cast(ulong)hash2) << 32);
        return hash;
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
            assert(axis >= 0 && axis < 3); // 0 - x axis, 1 - y axis, 2 - z axis
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

        void initLeafWithMultipleTriangles(int numTriangles, int triangleIndicesOffset)
        {
            assert(numTriangles > 1);
            header = leafNodeFlags | (numTriangles << 2); // == 11, 15, 19, ... (for numTriangles = 2, 3, 4, ...)
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

    unittest
    {
        assert(Node.sizeof == 8);
        {
            Node node;
            node.initInteriorNode(1, 123, 0.1f);
            assert(node.header == ((123 << 2) | 1));
            assert(node.split == 0.1f);
        }
        {
            Node node;
            node.initEmptyLeaf();
            assert(node.header == Node.leafNodeFlags);
            assert(node.index == 0);
        }
        {
            Node node;
            node.initLeafWithSingleTriangle(5);
            assert(node.header == 7);
            assert(node.index == 5);
        }
        {
            Node node;
            node.initLeafWithMultipleTriangles(6, 4);
            assert(node.header == ((6 << 2) | Node.leafNodeFlags));
            assert(node.index == 4);
        }
    }

public:
    immutable(Node)[] nodes;
    immutable(int)[] triangleIndices;

    enum traversalMaxDepth = 64;
    immutable(TriangleMesh) mesh;
    immutable(BoundingBox_f) meshBounds;
}
