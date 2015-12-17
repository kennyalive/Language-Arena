#pragma once

#include "bounding_box.h"
#include "kdtree.h"
#include <cstdint>
#include <vector>

class TriangleMesh;

class KdTreeBuildingException : public std::exception {
public:
    KdTreeBuildingException(const std::string& message);
    const char* what() const override;

private:
    const std::string _message;
};

class KdTreeBuilder {
public:
    struct BuildParams {
        float intersectionCost = 80;
        float traversalCost = 1;
        float emptyBonus = 0.5f;
        int leafCandidateTrianglesCount = 2;
        int maxDepth = -1;
        bool splitAlongTheLongestAxis = false;
        bool collectStats = false;
    };

    struct BuildStats {
        BuildStats(bool enabled);
        void updateTrianglesStack(int nodeTrianglesCount);
        void newLeaf(int leafTriangles, int depth);
        void finalizeStats();

        int32_t leafCount = 0;
        int32_t emptyLeafCount = 0;
        double trianglesPerLeaf = 0.0;
        int perfectDepth = 0;
        double averageDepth = 0.0;
        double depthStandardDeviation = 0.0;

    private:
        bool _enabled = true;
        int64_t _trianglesPerLeafAccumulated = 0;
        int64_t _leafDepthAccumulated = 0;
        std::vector<uint8_t> _leafDepthAppender;
        std::vector<int> _trianglesStack;
    };

public:
    KdTreeBuilder(const TriangleMesh& mesh, const BuildParams& buildParams);
    KdTree buildTree();
    const BuildStats& getBuildStats() const;

private:
    struct BoundEdge {
        float positionOnAxis;
        uint32_t triangleAndEndFlag;

        enum : uint32_t { endMask = 0x80000000 };
        enum : uint32_t { triangleMask = 0x7fffffff };

        bool isStart() const { return (triangleAndEndFlag & endMask) == 0; }
        bool isEnd() const { return !isStart(); }
        static bool less(BoundEdge edge1, BoundEdge edge2)
        {
            if (edge1.positionOnAxis == edge2.positionOnAxis)
                return edge1.isEnd() && edge2.isStart();
            else
                return edge1.positionOnAxis < edge2.positionOnAxis;
        }
    };

    struct Split {
        int32_t edge;
        int axis;
        float cost;
    };

private:
    void buildNode(const BoundingBox_f& nodeBounds,
                   const int32_t* nodeTriangles, int32_t nodeTrianglesCount,
                   int depth, int32_t* triangles0, int32_t* triangles1);

    void createLeaf(const int32_t* nodeTriangles, int32_t nodeTrianglesCount);

    Split selectSplit(const BoundingBox_f& nodeBounds,
                      const int32_t* nodeTriangles, int32_t nodeTrianglesCount);
    Split selectSplitForAxis(const BoundingBox_f& nodeBounds,
                             int32_t nodeTrianglesCount, int axis) const;

private:
    const TriangleMesh& _mesh;
    BuildParams _buildParams;
    BuildStats _buildStats;

    std::vector<BoundingBox_f> _triangleBounds;
    std::vector<BoundEdge> _edgesBuffer;
    std::vector<int32_t> _trianglesBuffer;

    std::vector<KdTree::Node> _nodes;
    std::vector<int32_t> _triangleIndices;
};
