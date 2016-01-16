#pragma once

#include "bounding_box.h"
#include "kdtree.h"
#include <cstdint>
#include <vector>

class TriangleMesh;

class KdTreeBuilder {
public:
  struct BuildParams;
  struct BuildStats;

  KdTreeBuilder(const TriangleMesh& mesh, const BuildParams& buildParams);

  KdTree BuildTree();
  const BuildStats& GetBuildStats() const;

public:
  struct BuildParams {
    float intersectionCost = 80;
    float traversalCost = 1;
    float emptyBonus = 0.3f;
    int leafCandidateTrianglesCount = 2;
    int maxDepth = -1;
    bool splitAlongTheLongestAxis = false;
    bool collectStats = true;
  };

  struct BuildStats {
    BuildStats(bool enabled);

    void UpdateTrianglesStack(int nodeTrianglesCount);
    void NewLeaf(int leafTriangles, int depth);
    void FinalizeStats(int32_t nodesCount, int32_t triangleIndicesCount);

    int32_t nodesCount = 0;
    int32_t leafCount = 0;
    int32_t emptyLeafCount = 0;
    int32_t triangleIndicesCount = 0;
    double trianglesPerLeaf = 0.0;
    int perfectDepth = 0;
    double averageDepth = 0.0;
    double depthStandardDeviation = 0.0;

  private:
    bool enabled = true;
    int64_t trianglesPerLeafAccumulated = 0;
    int64_t leafDepthAccumulated = 0;
    std::vector<uint8_t> leafDepthAppender;
    std::vector<int> trianglesStack;
  }; // BuildStats

private:
  struct BoundEdge {
    float positionOnAxis;
    uint32_t triangleAndEndFlag;

    enum : uint32_t { endMask = 0x80000000 };
    enum : uint32_t { triangleMask = 0x7fffffff };

    bool IsStart() const
    {
      return (triangleAndEndFlag & endMask) == 0;
    }

    bool IsEnd() const
    {
      return !IsStart();
    }

    static bool Less(BoundEdge edge1, BoundEdge edge2)
    {
      if (edge1.positionOnAxis == edge2.positionOnAxis)
        return edge1.IsEnd() && edge2.IsStart();
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
  void BuildNode(const BoundingBox_f& nodeBounds, const int32_t* nodeTriangles,
                 int32_t nodeTrianglesCount, int depth, int32_t* triangles0,
                 int32_t* triangles1);

  void CreateLeaf(const int32_t* nodeTriangles, int32_t nodeTrianglesCount);

  Split SelectSplit(const BoundingBox_f& nodeBounds,
                    const int32_t* nodeTriangles, int32_t nodeTrianglesCount);

  Split SelectSplitForAxis(const BoundingBox_f& nodeBounds,
                           int32_t nodeTrianglesCount, int axis) const;

private:
  const TriangleMesh& mesh;
  BuildParams buildParams;
  BuildStats buildStats;

  std::vector<BoundingBox_f> triangleBounds;
  std::vector<BoundEdge> edgesBuffer;
  std::vector<int32_t> trianglesBuffer;

  std::vector<KdTree::Node> nodes;
  std::vector<int32_t> triangleIndices;
};
