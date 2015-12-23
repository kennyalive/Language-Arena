#include "kdtree.h"
#include "triangle.h"

#include <cassert>
#include <fstream>

KdTree::KdTree(std::vector<Node>&& nodes,
               std::vector<int32_t>&& triangleIndices, const TriangleMesh& mesh,
               const BoundingBox_f& meshBounds)
    : _nodes(std::move(nodes)), _triangleIndices(std::move(triangleIndices)),
      _mesh(mesh), _meshBounds(meshBounds)
{
}

KdTree::KdTree(KdTree&& other)
    : _nodes(std::move(other._nodes)),
      _triangleIndices(std::move(other._triangleIndices)), _mesh(other._mesh),
      _meshBounds(other._meshBounds)
{
}

KdTree::KdTree(const std::string& kdtreeFileName, const TriangleMesh& mesh)
    : _mesh(mesh), _meshBounds(mesh.getBounds())
{
  std::ifstream file(kdtreeFileName, std::ios_base::in | std::ios_base::binary);
  if (!file)
    throw "KdTree: failed to load kdtree file";

  int32_t nodesCount;
  file.read(reinterpret_cast<char*>(&nodesCount), 4);
  if (!file)
    throw "KdTree: failed to read nodes count";

  auto& nodes = const_cast<std::vector<Node>&>(_nodes);
  nodes.resize(nodesCount);
  file.read(reinterpret_cast<char*>(nodes.data()), nodesCount * sizeof(Node));
  if (!file)
    throw "KdTree: failed to read kdtree nodes";

  int32_t triangleIndicesCount;
  file.read(reinterpret_cast<char*>(&triangleIndicesCount), 4);
  if (!file)
    throw "KdTree: failed to read triangle indices count";

  auto& triangleIndices = const_cast<std::vector<int32_t>&>(_triangleIndices);
  triangleIndices.resize(triangleIndicesCount);
  file.read(reinterpret_cast<char*>(triangleIndices.data()),
            triangleIndicesCount * 4);
  if (!file)
    throw "KdTree: failed to read triangle indices";
}

bool KdTree::intersect(const Ray& ray, Intersection& intersection) const
{
  auto boundsIntersection = _meshBounds.intersect(ray);
  if (!boundsIntersection.found)
    return false;

  struct TraversalInfo {
    const Node* node;
    double tMin;
    double tMax;
  };
  TraversalInfo traversalStack[maxTraversalDepth];
  int traversalStackSize = 0;

  double tMin = boundsIntersection.t0;
  double tMax = boundsIntersection.t1;

  auto node = &_nodes[0];

  RayTriangleIntersection closestIntersection;
  closestIntersection.t = std::numeric_limits<double>::infinity();

  while (closestIntersection.t > tMin) {
    if (node->isInteriorNode()) {
      int axis = node->getInteriorNodeSplitAxis();
      double distanceToSplitPlane = node->split - ray.getOrigin()[axis];

      if (distanceToSplitPlane == 0.0) {
        if (ray.getDirection()[axis] > 0.0)
          node = &_nodes[node->getInteriorNodeAboveChild()];
        else
          node += 1;
        // todo: check case when ray.direction[axis] == 0.0, probably in this
        // case we
        // we need to process both nodes to avoid rendering artifacts (although
        // rare cases)
      }
      else {
        const Node* firstChild;
        const Node* secondChild;

        if (distanceToSplitPlane > 0.0) {
          firstChild = node + 1;
          secondChild = &_nodes[node->getInteriorNodeAboveChild()];
        }
        else {
          firstChild = &_nodes[node->getInteriorNodeAboveChild()];
          secondChild = node + 1;
        }

        double tSplit =
            distanceToSplitPlane * ray.getInvDirection()[axis]; // != 0
        if (tSplit >= tMax || tSplit < 0.0)
          node = firstChild;
        else if (tSplit <= tMin)
          node = secondChild;
        else // tMin < tSplit < tMax
        {
          assert(traversalStackSize < maxTraversalDepth);
          traversalStack[traversalStackSize++] = {secondChild, tSplit, tMax};
          node = firstChild;
          tMax = tSplit;
        }
      }
    }
    else // leaf node
    {
      int trianglesCount = node->getLeafTrianglesCount();
      if (trianglesCount == 1) {
        auto indices = _mesh.triangles[node->index].vertexIndices;
        Triangle triangle;
        triangle.p[0] = Vector(_mesh.vertices[indices[0]]);
        triangle.p[1] = Vector(_mesh.vertices[indices[1]]);
        triangle.p[2] = Vector(_mesh.vertices[indices[2]]);

        RayTriangleIntersection triangleIntersection;
        bool intersectionFound =
            intersectTriangle(ray, triangle, triangleIntersection);
        if (intersectionFound &&
            triangleIntersection.t < closestIntersection.t) {
          closestIntersection = triangleIntersection;
        }
      }
      else {
        for (int32_t i = 0; i < trianglesCount; ++i) {
          auto indices =
              _mesh.triangles[_triangleIndices[node->index + i]].vertexIndices;
          Triangle triangle;
          triangle.p[0] = Vector(_mesh.vertices[indices[0]]);
          triangle.p[1] = Vector(_mesh.vertices[indices[1]]);
          triangle.p[2] = Vector(_mesh.vertices[indices[2]]);

          RayTriangleIntersection triangleIntersection;
          bool intersectionFound =
              intersectTriangle(ray, triangle, triangleIntersection);
          if (intersectionFound &&
              triangleIntersection.t < closestIntersection.t) {
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

  bool isHit = closestIntersection.t < std::numeric_limits<double>::infinity();
  if (isHit) {
    intersection.rayT = closestIntersection.t;
    intersection.rayTEpsilon = closestIntersection.rayEpsilon;
  }
  return isHit;
}

const TriangleMesh& KdTree::getMesh() const { return _mesh; }

const BoundingBox_f& KdTree::getMeshBounds() const { return _meshBounds; }

void KdTree::saveToFile(const std::string& fileName) const
{
  std::ofstream file(fileName, std::ios_base::out | std::ios_base::binary);
  if (!file)
    throw "Failed to write kdtree to file " + fileName;

  int32_t nodesCount = static_cast<int32_t>(_nodes.size());
  file.write(reinterpret_cast<const char*>(&nodesCount), 4);
  file.write(reinterpret_cast<const char*>(_nodes.data()),
             _nodes.size() * sizeof(Node));

  int32_t triangleIndicesCount = static_cast<int32_t>(_triangleIndices.size());
  file.write(reinterpret_cast<const char*>(&triangleIndicesCount), 4);
  file.write(reinterpret_cast<const char*>(_triangleIndices.data()),
             _triangleIndices.size() * 4);
}

uint64_t KdTree::getHash() const
{
  uint32_t hash1 = 0;
  for (const auto& node : _nodes) {
    hash1 = hash1 * 33 + node.header + node.index;
  }

  uint32_t hash2 = 0;
  for (int32_t index : _triangleIndices) {
    hash2 = hash2 * 33 + uint32_t(index);
  }

  uint64_t hash = hash1 + (static_cast<uint64_t>(hash2) << 32);
  return hash;
}
