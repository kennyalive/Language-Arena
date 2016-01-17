import std.algorithm;
import std.array;
import std.conv;
import std.exception;
import std.math;
import std.range;
import std.typecons;

import bounding_box;
import common;
import kdtree;
import triangle_mesh;
import vector;

struct KdTreeBuilder
{
    struct BuildParams
    {
        float intersectionCost = 80;
        float traversalCost = 1;
        float emptyBonus = 0.3f;
        int maxDepth = -1;
        bool splitAlongTheLongestAxis = false;
        // the actual amout of leaf triangles can be larger
        int leafTrianglesLimit = 2;
        bool collectStats = true;
    }

    struct BuildStats
    {
        pure: nothrow:

        this(bool enabled)
        {
            this.enabled = enabled;
        }

        void newLeaf(int leafTriangles, int depth)
        {
            if (!enabled)
                return;

            ++_leafCount;

            if (leafTriangles == 0)
            {
                ++_emptyLeafCount;
            }
            else // not empty leaf
            {
                leafDepthAccumulated += depth;
                leafDepthAppender.put(cast(ubyte)depth);
                trianglesPerLeafAccumulated += leafTriangles;
            }
        }

        void finalizeStats()
        {
            if (!enabled)
                return;

            auto notEmptyLeafCount = _leafCount - _emptyLeafCount;

            _trianglesPerLeaf = cast(double)trianglesPerLeafAccumulated / notEmptyLeafCount;

            _perfectDepth = cast(int)ceil(log2(_leafCount));
            _averageDepth = cast(double)leafDepthAccumulated / notEmptyLeafCount;

            double accum = 0.0;
            foreach (depth; leafDepthAppender.data)
            {
                auto diff = depth - _averageDepth;
                accum += diff*diff;
            }
            _depthStandardDeviation = sqrt(accum / notEmptyLeafCount);
        }

        @property int leafCount() const 
        {
            return _leafCount;
        }

        @property int emptyLeafCount() const
        {
            return _emptyLeafCount;
        }

        @property double trianglesPerLeaf() const
        {
            return _trianglesPerLeaf;
        }

        @property int perfectDepth() const
        {
            return _perfectDepth;
        }

        @property double averageDepth() const
        {
            return _averageDepth;
        }

        @property double depthStandardDeviation() const
        {
            return _depthStandardDeviation;
        }

    private:
        bool enabled = true;
        long trianglesPerLeafAccumulated = 0;
        long leafDepthAccumulated = 0;
        Appender!(ubyte[]) leafDepthAppender;

        int _leafCount = 0;
        int _emptyLeafCount = 0;
        double _trianglesPerLeaf;
        int _perfectDepth;
        double _averageDepth;
        double _depthStandardDeviation;
    }

    // max count is chosen such that maxTrianglesCount * 2 is still an int, 
    // this simplifies implementation.
    private enum maxTrianglesCount = 0x3fff_ffff; // max ~ 1 billion triangles

    this(immutable(TriangleMesh) mesh, BuildParams buildParams)
    {
        if (mesh.triangles.length > maxTrianglesCount)
            runtimeError("exceeded the maximum number of mesh triangles: " ~ 
                to!string(maxTrianglesCount));

        this.mesh = mesh;

        if (buildParams.maxDepth <= 0)
        {
            buildParams.maxDepth = cast(int)
                (0.5 + (8.0 + 1.3 * floor(log2(mesh.getTrianglesCount()))));
        }
        buildParams.maxDepth = min(buildParams.maxDepth, KdTree.traversalMaxDepth);

        this.buildParams = buildParams;
        this.buildStats = BuildStats(buildParams.collectStats);
    }

    KdTree buildTree()
    {
        // initialize bounding boxes
        auto trianglesBoundsAppender = appender!(BoundingBox_f[]);
        BoundingBox_f meshBounds;
        foreach (i; 0..mesh.getTrianglesCount())
        {
            auto bounds = mesh.getTriangleBounds(i);
            trianglesBoundsAppender.put(bounds);
            meshBounds = boundsUnion(meshBounds, bounds);
        }
        trianglesBounds = trianglesBoundsAppender.data;

        // allocate working memory
        edgesBuffer = new BoundEdge[2 * mesh.getTrianglesCount()];
        trianglesBuffer = new int[mesh.getTrianglesCount() * 
            (buildParams.maxDepth + 1)];

        // fill triangle indices for root node
        trianglesBuffer[0..mesh.getTrianglesCount()] = 
            array(iota(0, mesh.getTrianglesCount()));

        // recursively build all nodes
        buildNode(meshBounds, trianglesBuffer[0..mesh.getTrianglesCount()], 
                    buildParams.maxDepth, trianglesBuffer.ptr, 
                    trianglesBuffer.ptr + mesh.getTrianglesCount());

        buildStats.finalizeStats();
        return new KdTree(assumeUnique(nodesAppender.data), 
            assumeUnique(triangleIndicesAppender.data), mesh);
    }

    nothrow
    const(BuildStats) GetBuildStats() const
    {
        return buildStats;
    }

private:
    struct BoundEdge
    {
        pure: nothrow: @nogc:

        float positionOnAxis;
        uint triangleAndEndFlag;

        enum uint endMask = 0x8000_0000;
        enum uint triangleMask = 0x7fff_ffff;

        bool isStart() const
        {
            return (triangleAndEndFlag & endMask) == 0;
        }

        bool isEnd() const
        {
            return !isStart;
        }

        static bool less(BoundEdge edge1, BoundEdge edge2)
        {
            if (edge1.positionOnAxis == edge2.positionOnAxis)
                return edge1.isEnd() && edge2.isStart();
            else 
                return edge1.positionOnAxis < edge2.positionOnAxis;
        }
    }

    void buildNode(BoundingBox_f nodeBounds, const(int[]) nodeTriangles,
        int depth, int* triangles0, int* triangles1)
    {
        if (nodesAppender.data.length >= KdTree.Node.maxNodesCount)
            runtimeError("The maximum number of KdTree nodes has been reached: " ~
                to!string(KdTree.Node.maxNodesCount));

        auto nodeTrianglesCount = cast(int)nodeTriangles.length;

        // check if leaf node should be created
        if (nodeTriangles.length <= buildParams.leafTrianglesLimit || depth == 0)
        {
            createLeaf(nodeTriangles);
            buildStats.newLeaf(cast(int) nodeTriangles.length,
                buildParams.maxDepth - depth);
            return;
        }

        // select split position
        auto split = selectSplit(nodeBounds, nodeTriangles);
        if (split.edge == -1)
        {
            createLeaf(nodeTriangles);
            buildStats.newLeaf(cast(int) nodeTriangles.length,
                buildParams.maxDepth - depth);
            return;
        }
        float splitPosition = edgesBuffer[split.edge].positionOnAxis;

        // classify triangles with respect to split
        int n0 = 0;
        foreach (i; 0..split.edge)
        {
            if (edgesBuffer[i].isStart())
            {
                triangles0[n0++] = edgesBuffer[i].triangleAndEndFlag;
            }
        }

        int n1 = 0;
        foreach (i; split.edge + 1 .. nodeTriangles.length*2)
        {
            if (edgesBuffer[i].isEnd())
            {
                int triangle = edgesBuffer[i].triangleAndEndFlag & BoundEdge.triangleMask;
                triangles1[n1++] = triangle;
            }
        }

        // add interior node and recursively create children nodes
        int thisNodeIndex =cast(int) nodesAppender.data.length;
        nodesAppender.put(KdTree.Node());

        BoundingBox_f bounds0 = nodeBounds;
        bounds0.maxPoint[split.axis] = splitPosition;
        buildNode(bounds0, triangles0[0..n0], depth - 1, triangles0, 
            triangles1 + n1);

        int aboveChild = cast(int)nodesAppender.data.length;
        nodesAppender.data[thisNodeIndex].initInteriorNode(split.axis, 
            aboveChild, splitPosition);

        BoundingBox_f bounds1 = nodeBounds;
        bounds1.minPoint[split.axis] = splitPosition;
        buildNode(bounds1, triangles1[0..n1], depth - 1, triangles0, triangles1);
    }

    void createLeaf(const(int[]) nodeTriangles)
    {
        KdTree.Node node;
        if (nodeTriangles.length == 0) 
        {
            node.initEmptyLeaf();
        }
        else if (nodeTriangles.length == 1)
        {
            node.initLeafWithSingleTriangle(nodeTriangles[0]);
        }
        else
        {
            node.initLeafWithMultipleTriangles(cast(int)nodeTriangles.length, 
                cast(int)triangleIndicesAppender.data.length);
            triangleIndicesAppender.put(nodeTriangles);
        }
        nodesAppender.put(node);
    }

    alias Split = Tuple!(int, "edge", int, "axis", float, "cost");

    Split selectSplit(BoundingBox_f nodeBounds, const(int[]) nodeTriangles)
    {
        // Determine axes iteration order.
        int[3] axes;
        if (buildParams.splitAlongTheLongestAxis)
        {
            Vector_f diag = nodeBounds.maxPoint - nodeBounds.minPoint;
            if (diag.x >= diag.y && diag.x >= diag.z)
            {
                axes[0] = 0;
                axes[1] = diag.y >= diag.z ? 1 : 2;
            }
            else if (diag.y >= diag.x && diag.y >= diag.z)
            {
                axes[0] = 1;
                axes[1] = diag.x >= diag.z ? 0 : 2;
            }
            else
            {
                axes[0] = 2;
                axes[1] = diag.x >= diag.y ? 0 : 1;
            }
            axes[2] = 3 - axes[0] - axes[1]; // since 0 + 1 + 2 == 3
        }
        else
        {
            axes[0] = 0;
            axes[1] = 1;
            axes[2] = 2;
        }

        // Select spliting axis and position. 
        // If buildParams.splitAlongTheLongestAxis is true then we stop at the 
        // first axis that gives a valid split.
        auto bestSplit = Split(-1, -1, float.infinity);
        foreach (index; 0..3)
        {
            int axis = axes[index];

            // initialize edges
            foreach (i; 0..nodeTriangles.length)
            {
                int triangle = nodeTriangles[i];
                edgesBuffer[2*i + 0] = BoundEdge(trianglesBounds[triangle].minPoint[axis], triangle);
                edgesBuffer[2*i + 1] = BoundEdge(trianglesBounds[triangle].maxPoint[axis], triangle | BoundEdge.endMask);
            }
            sort!(BoundEdge.less, SwapStrategy.stable)(edgesBuffer[0..nodeTriangles.length*2]);

            // select split position
            auto split = selectSplitForAxis(nodeBounds, cast(int)nodeTriangles.length, axis);
            if (split.edge != -1)
            {
                if (buildParams.splitAlongTheLongestAxis)
                    return split;
                if (split.cost < bestSplit.cost)
                    bestSplit = split;
            }
        }

        // If split axis is not the last axis (2) then we should reinitialize edgesBuffer to 
        // contain data for split axis since edgesBuffer will be used later.
        if (bestSplit.axis == 0 || bestSplit.axis == 1)
        {
            foreach (i; 0..nodeTriangles.length)
            {
                int triangle = nodeTriangles[i];
                edgesBuffer[2*i + 0] = BoundEdge(trianglesBounds[triangle].minPoint[bestSplit.axis], triangle);
                edgesBuffer[2*i + 1] = BoundEdge(trianglesBounds[triangle].maxPoint[bestSplit.axis], triangle | BoundEdge.endMask);
            }
            sort!(BoundEdge.less, SwapStrategy.stable)(edgesBuffer[0..nodeTriangles.length*2]);
        }
        return bestSplit;
    }

    nothrow @nogc
    Split selectSplitForAxis(BoundingBox_f nodeBounds, int nodeTrianglesCount, int axis)
    {
        static immutable(int[2][3]) otherAxis = [ [1, 2], [0, 2], [0, 1] ];
        immutable(int) otherAxis0 = otherAxis[axis][0];
        immutable(int) otherAxis1 = otherAxis[axis][1];
        immutable(Vector_f) diag = nodeBounds.maxPoint - nodeBounds.minPoint;

        immutable(float) s0 = 2.0f * (diag[otherAxis0] * diag[otherAxis1]);
        immutable(float) d0 = 2.0f * (diag[otherAxis0] + diag[otherAxis1]);
        immutable(float) invTotalS = 1.0f / (2.0f * (diag.x*diag.y + diag.x*diag.z + diag.y*diag.z));
        immutable(int) numEdges = nodeTrianglesCount * 2;

        auto bestSplit = Split(-1, axis, buildParams.intersectionCost * nodeTrianglesCount);

        int numBelow = 0;
        int numAbove = nodeTrianglesCount;

        int i = 0;
        while (i < numEdges)
        {
            BoundEdge edge = edgesBuffer[i];

            // find group of edges with the same axis position: [i, groupEnd)
            int groupEnd = i + 1;
            while (groupEnd < numEdges && edge.positionOnAxis == edgesBuffer[groupEnd].positionOnAxis)
                ++groupEnd;

            // [i, middleEdge) - edges End points.
            // [middleEdge, groupEnd) - edges Start points.
            int middleEdge = i;
            while (middleEdge != groupEnd && edgesBuffer[middleEdge].isEnd())
                ++middleEdge;

            numAbove -= middleEdge - i;

            float t = edge.positionOnAxis;
            if (t > nodeBounds.minPoint[axis] && t < nodeBounds.maxPoint[axis])
            {
                auto belowS = s0 + d0 * (t - nodeBounds.minPoint[axis]);
                auto aboveS = s0 + d0 * (nodeBounds.maxPoint[axis] - t);

                auto pBelow = belowS * invTotalS;
                auto pAbove = aboveS * invTotalS;

                auto emptyBonus = (numBelow == 0 || numAbove == 0) ? buildParams.emptyBonus : 0.0f;

                auto cost = buildParams.traversalCost + 
                    (1.0f - emptyBonus) * buildParams.intersectionCost * (pBelow*numBelow + pAbove*numAbove);

                if (cost < bestSplit.cost)
                {
                    bestSplit.edge = (middleEdge == groupEnd) ?  middleEdge - 1 : middleEdge;
                    bestSplit.cost = cost;
                }
            }

            numBelow += groupEnd - middleEdge;
            i = groupEnd;
        }
        return bestSplit;
    }

private:
    immutable(TriangleMesh) mesh;
    immutable(BuildParams) buildParams;
    BuildStats buildStats;

    BoundingBox_f[] trianglesBounds;
    BoundEdge[] edgesBuffer;
    int[] trianglesBuffer;

    Appender!(KdTree.Node[]) nodesAppender;
    Appender!(int[]) triangleIndicesAppender;
}
