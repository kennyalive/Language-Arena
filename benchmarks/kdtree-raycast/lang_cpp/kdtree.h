#pragma once

#include "bounding_box.h"
#include "ray.h"
#include "triangle.h"
#include "triangle_mesh.h"
#include "vector.h"
#include <cassert>
#include <cstdint>
#include <vector>

class KdTree {
  struct Node;

  KdTree(const KdTree&) = delete;
  KdTree& operator=(const KdTree&) = delete;

public:
  KdTree(std::vector<Node>&& nodes, std::vector<int32_t>&& triangleIndices,
         const TriangleMesh& mesh, const BoundingBox_f& meshBounds);
  KdTree(const std::string& kdtreeFileName, const TriangleMesh& mesh);
  void SaveToFile(const std::string& fileName) const;

  struct Intersection {
    double t = std::numeric_limits<double>::infinity();
    double epsilon = 0.0;
  };

  bool Intersect(const Ray& ray, Intersection& intersection) const;

  const TriangleMesh& GetMesh() const;
  const BoundingBox_f& GetMeshBounds() const;

private:
  void IntersectLeafTriangles(
      const Ray& ray, Node leaf,
      Triangle::Intersection& closestIntersection) const;

private:
  friend class KdTreeBuilder;

  enum { maxTraversalDepth = 64 };

  struct Node {
    uint32_t header;
    union {
      float split;
      int32_t index;
    };

    enum : int32_t { maxNodesCount = 0x40000000 };
    enum : uint32_t { leafNodeFlags = 3 };

    void initInteriorNode(int axis, int32_t aboveChild, float split)
    {
      assert(axis >= 0 && axis < 3); // 0 - x axis, 1 - y axis, 2 - z axis
      assert(aboveChild < maxNodesCount);
      header = axis | (aboveChild << 2);
      this->split = split;
    }

    void initEmptyLeaf()
    {
      header = leafNodeFlags; // = 3
      index = 0;              // not used for empty leaf, just set default value
    }

    void initLeafWithSingleTriangle(int triangleIndex)
    {
      header = leafNodeFlags | (1 << 2); // = 7
      index = triangleIndex;
    }

    void initLeafWithMultipleTriangles(int32_t numTriangles,
                                       int32_t triangleIndicesOffset)
    {
      assert(numTriangles > 1);
      header = leafNodeFlags |
               (numTriangles
                << 2); // == 11, 15, 19, ... (for numTriangles = 2, 3, 4, ...)
      index = triangleIndicesOffset;
    }

    bool isLeaf() const { return (header & leafNodeFlags) == leafNodeFlags; }

    bool isInteriorNode() const { return !isLeaf(); }

    int getLeafTrianglesCount() const
    {
      assert(isLeaf());
      return header >> 2;
    }

    int getInteriorNodeSplitAxis() const
    {
      assert(isInteriorNode());
      return header & leafNodeFlags;
    }

    int getInteriorNodeAboveChild() const
    {
      assert(isInteriorNode());
      return header >> 2;
    }
  };

private:
  const std::vector<Node> _nodes;
  const std::vector<int32_t> _triangleIndices;

  const TriangleMesh& _mesh;
  const BoundingBox_f _meshBounds;
};
