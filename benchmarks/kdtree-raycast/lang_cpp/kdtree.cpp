#include "common.h"
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

KdTree::KdTree(const std::string& kdTreeFileName, const TriangleMesh& mesh)
    : _mesh(mesh), _meshBounds(mesh.GetBounds())
{
  std::ifstream file(kdTreeFileName, std::ios_base::in | std::ios_base::binary);
  if (!file)
    RuntimeError("failed to open kdTree file: " + kdTreeFileName);

  int32_t nodesCount;
  file.read(reinterpret_cast<char*>(&nodesCount), 4);
  if (!file)
    RuntimeError("failed to read nodes count: " + kdTreeFileName);

  auto& nodes = const_cast<std::vector<Node>&>(_nodes);
  nodes.resize(nodesCount);
  file.read(reinterpret_cast<char*>(nodes.data()), nodesCount * sizeof(Node));
  if (!file)
    RuntimeError("failed to read kdTree nodes: " + kdTreeFileName);

  int32_t triangleIndicesCount;
  file.read(reinterpret_cast<char*>(&triangleIndicesCount), 4);
  if (!file)
    RuntimeError("failed to read triangle indices count: " + kdTreeFileName);

  auto& triangleIndices = const_cast<std::vector<int32_t>&>(_triangleIndices);
  triangleIndices.resize(triangleIndicesCount);
  file.read(reinterpret_cast<char*>(triangleIndices.data()),
            triangleIndicesCount * 4);
  if (!file)
    RuntimeError("failed to read kdTree triangle indices: " + kdTreeFileName);
}

void KdTree::SaveToFile(const std::string& fileName) const
{
  std::ofstream file(fileName, std::ios_base::out | std::ios_base::binary);
  if (!file)
    RuntimeError("failed to open kdTree file for writing: " + fileName);

  int32_t nodesCount = static_cast<int32_t>(_nodes.size());
  file.write(reinterpret_cast<const char*>(&nodesCount), 4);
  file.write(reinterpret_cast<const char*>(_nodes.data()),
             _nodes.size() * sizeof(Node));
  if (!file)
    RuntimeError("failed to write kdTree nodes: " + fileName);

  int32_t triangleIndicesCount = static_cast<int32_t>(_triangleIndices.size());
  file.write(reinterpret_cast<const char*>(&triangleIndicesCount), 4);
  file.write(reinterpret_cast<const char*>(_triangleIndices.data()),
             _triangleIndices.size() * 4);
  if (!file)
    RuntimeError("failed to write kdTree triangle indices: " + fileName);
}

bool KdTree::Intersect(const Ray& ray, RayIntersection& intersection) const
{
  auto boundsIntersection = _meshBounds.Intersect(ray);
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

  Triangle::Intersection closestIntersection;
  closestIntersection.t = std::numeric_limits<double>::infinity();

  while (closestIntersection.t > tMin) {
    if (node->isInteriorNode()) {
      int axis = node->getInteriorNodeSplitAxis();
      double distanceToSplitPlane = node->split - ray.getOrigin()[axis];
      auto belowChild = node + 1;
      auto aboveChild = &_nodes[node->getInteriorNodeAboveChild()];

      if (distanceToSplitPlane != 0.0) { // general case
        const Node *firstChild, *secondChild;

        if (distanceToSplitPlane > 0.0) {
          firstChild = belowChild;
          secondChild = aboveChild;
        }
        else {
          firstChild = aboveChild;
          secondChild = belowChild;
        }

        // tSplit != 0 (since distanceToSplitPlane != 0)
        double tSplit = distanceToSplitPlane * ray.getInvDirection()[axis];
        if (tSplit >= tMax || tSplit < 0.0)
          node = firstChild;
        else if (tSplit <= tMin)
          node = secondChild;
        else { // tMin < tSplit < tMax
          assert(traversalStackSize < maxTraversalDepth);
          traversalStack[traversalStackSize++] = {secondChild, tSplit, tMax};
          node = firstChild;
          tMax = tSplit;
        }
      }
      else { // special case, distanceToSplitPlane == 0.0
        if (ray.getDirection()[axis] > 0.0) {
          if (tMin > 0.0)
            node = aboveChild;
          else { // tMin == 0.0
            assert(traversalStackSize < maxTraversalDepth);
            traversalStack[traversalStackSize++] = {aboveChild, 0.0, tMax};
            // check single point [0.0, 0.0]
            node = belowChild;
            tMax = 0.0;
          }
        }
        else if (ray.getDirection()[axis] < 0.0) {
          if (tMin > 0.0)
            node = belowChild;
          else { // tMin == 0.0
            assert(traversalStackSize < maxTraversalDepth);
            traversalStack[traversalStackSize++] = {belowChild, 0.0, tMax};
            // check single point [0.0, 0.0]
            node = aboveChild;
            tMax = 0.0;
          }
        }
        else { // ray.direction[axis] == 0.0
          // for both nodes check [tMin, tMax] range
          assert(traversalStackSize < maxTraversalDepth);
          traversalStack[traversalStackSize++] = {aboveChild, tMin, tMax};
          node = belowChild;
        }
      }
    }
    else { // leaf node
      IntersectLeafTriangles(ray, *node, closestIntersection);

      if (traversalStackSize == 0)
        break;

      --traversalStackSize;
      node = traversalStack[traversalStackSize].node;
      tMin = traversalStack[traversalStackSize].tMin;
      tMax = traversalStack[traversalStackSize].tMax;
    }
  } // while (closestIntersection.t > tMin)

  if (closestIntersection.t == std::numeric_limits<double>::infinity())
    return false;

  intersection.t = closestIntersection.t;
  intersection.epsilon = closestIntersection.epsilon;
  return true;
}

void KdTree::IntersectLeafTriangles(
    const Ray& ray, Node leaf,
    Triangle::Intersection& closestIntersection) const
{
  int trianglesCount = leaf.getLeafTrianglesCount();
  if (trianglesCount == 1) {
    const auto& p = _mesh.triangles[leaf.index].points;

    Triangle triangle = {{Vector(_mesh.vertices[p[0].vertexIndex]),
                          Vector(_mesh.vertices[p[1].vertexIndex]),
                          Vector(_mesh.vertices[p[2].vertexIndex])}};

    Triangle::Intersection triangleIntersection;
    bool intersectionFound =
        IntersectTriangle(ray, triangle, triangleIntersection);
    if (intersectionFound && triangleIntersection.t < closestIntersection.t) {
      closestIntersection = triangleIntersection;
    }
  }
  else {
    for (int32_t i = 0; i < trianglesCount; ++i) {
      const auto& p = _mesh.triangles[_triangleIndices[leaf.index + i]].points;

      Triangle triangle = {{Vector(_mesh.vertices[p[0].vertexIndex]),
                            Vector(_mesh.vertices[p[1].vertexIndex]),
                            Vector(_mesh.vertices[p[2].vertexIndex])}};

      Triangle::Intersection triangleIntersection;
      bool intersectionFound =
          IntersectTriangle(ray, triangle, triangleIntersection);
      if (intersectionFound && triangleIntersection.t < closestIntersection.t) {
        closestIntersection = triangleIntersection;
      }
    }
  }
}

const TriangleMesh& KdTree::GetMesh() const { return _mesh; }

const BoundingBox_f& KdTree::GetMeshBounds() const { return _meshBounds; }
