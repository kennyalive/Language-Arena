#include "common.h"
#include "kdtree.h"
#include "triangle.h"
#include <cassert>
#include <fstream>

KdTree::KdTree(std::vector<Node>&& nodes,
               std::vector<int32_t>&& triangleIndices, const TriangleMesh& mesh)
: nodes(std::move(nodes))
, triangleIndices(std::move(triangleIndices))
, mesh(mesh)
, meshBounds(mesh.GetBounds())
{
}

KdTree::KdTree(const std::string& fileName, const TriangleMesh& mesh)
: mesh(mesh)
, meshBounds(mesh.GetBounds())
{
  std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);
  if (!file)
    RuntimeError("failed to open kdTree file: " + fileName);

  // read nodes
  int32_t nodesCount;
  file.read(reinterpret_cast<char*>(&nodesCount), 4);
  if (!file)
    RuntimeError("failed to read nodes count: " + fileName);

  auto& mutableNodes = const_cast<std::vector<Node>&>(nodes);
  mutableNodes.resize(nodesCount);

  auto nodesBytesCount = nodesCount * sizeof(Node);
  file.read(reinterpret_cast<char*>(mutableNodes.data()), nodesBytesCount);
  if (!file)
    RuntimeError("failed to read kdTree nodes: " + fileName);

  // read triangle indices
  int32_t indicesCount;
  file.read(reinterpret_cast<char*>(&indicesCount), 4);
  if (!file)
    RuntimeError("failed to read triangle indices count: " + fileName);

  auto& mutableIndices = const_cast<std::vector<int32_t>&>(triangleIndices);
  mutableIndices.resize(indicesCount);

  auto indicesBytesCount = indicesCount * 4;
  file.read(reinterpret_cast<char*>(mutableIndices.data()), indicesBytesCount);
  if (!file)
    RuntimeError("failed to read kdTree triangle indices: " + fileName);
}

void KdTree::SaveToFile(const std::string& fileName) const
{
  std::ofstream file(fileName, std::ios_base::out | std::ios_base::binary);
  if (!file)
    RuntimeError("failed to open kdTree file for writing: " + fileName);

  // write nodes
  int32_t nodesCount = static_cast<int32_t>(nodes.size());
  file.write(reinterpret_cast<const char*>(&nodesCount), 4);

  auto nodesBytesCount = nodesCount * sizeof(Node);
  file.write(reinterpret_cast<const char*>(nodes.data()), nodesBytesCount);
  if (!file)
    RuntimeError("failed to write kdTree nodes: " + fileName);

  // write triangle indices
  int32_t indicesCount = static_cast<int32_t>(triangleIndices.size());
  file.write(reinterpret_cast<const char*>(&indicesCount), 4);

  auto indicesBytesCount = indicesCount * 4;
  file.write(reinterpret_cast<const char*>(triangleIndices.data()),
             indicesBytesCount);
  if (!file)
    RuntimeError("failed to write kdTree triangle indices: " + fileName);
}

bool KdTree::Intersect(const Ray& ray, Intersection& intersection) const
{
  auto boundsIntersection = meshBounds.Intersect(ray);
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

  Triangle::Intersection closestIntersection;
  auto node = &nodes[0];

  while (closestIntersection.t > tMin) {
    if (node->IsInteriorNode()) {
      int axis = node->GetSplitAxis();

      double distanceToSplitPlane =
          node->GetSplitPosition() - ray.GetOrigin()[axis];

      auto belowChild = node + 1;
      auto aboveChild = &nodes[node->GetAboveChild()];

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
        double tSplit = distanceToSplitPlane * ray.GetInvDirection()[axis];
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
        if (ray.GetDirection()[axis] > 0.0) {
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
        else if (ray.GetDirection()[axis] < 0.0) {
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
  if (leaf.GetTrianglesCount() == 1) {
    const auto& p = mesh.triangles[leaf.GetIndex()].points;

    Triangle triangle = {{Vector(mesh.vertices[p[0].vertexIndex]),
                          Vector(mesh.vertices[p[1].vertexIndex]),
                          Vector(mesh.vertices[p[2].vertexIndex])}};

    Triangle::Intersection intersection;
    bool hitFound = IntersectTriangle(ray, triangle, intersection);
    if (hitFound && intersection.t < closestIntersection.t) {
      closestIntersection = intersection;
    }
  }
  else {
    for (int32_t i = 0; i < leaf.GetTrianglesCount(); i++) {
      int32_t triangleIndex = triangleIndices[leaf.GetIndex() + i];
      const auto& p = mesh.triangles[triangleIndex].points;

      Triangle triangle = {{Vector(mesh.vertices[p[0].vertexIndex]),
                            Vector(mesh.vertices[p[1].vertexIndex]),
                            Vector(mesh.vertices[p[2].vertexIndex])}};

      Triangle::Intersection intersection;
      bool hitFound = IntersectTriangle(ray, triangle, intersection);
      if (hitFound && intersection.t < closestIntersection.t) {
        closestIntersection = intersection;
      }
    }
  }
}

const TriangleMesh& KdTree::GetMesh() const
{
  return mesh;
}

const BoundingBox& KdTree::GetMeshBounds() const
{
  return meshBounds;
}

size_t KdTree::GetHash() const
{
  uint32_t hash1 = 0;
  for (const auto& node : nodes) {
    hash1 = hash1 * 33 + node.word0 + node.word1;
  }

  uint32_t hash2 = 0;
  for (int32_t index : triangleIndices) {
    hash2 = hash2 * 33 + uint32_t(index);
  }

  uint64_t hash = hash1 + (static_cast<uint64_t>(hash2) << 32);
  return hash;
}
