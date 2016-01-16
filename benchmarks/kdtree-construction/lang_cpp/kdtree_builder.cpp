#include "kdtree_builder.h"
#include "triangle_mesh.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

enum {
  // max count is chosen such that maxTrianglesCount * 2 is still an int32_t,
  // this simplifies implementation.
  maxTrianglesCount = 0x3fffffff // max ~ 1 billion triangles
};

KdTreeBuildingException::KdTreeBuildingException(const std::string& message)
: _message(message)
{
}

const char* KdTreeBuildingException::what() const
{
  return _message.c_str();
}
KdTreeBuilder::KdTreeBuilder(const TriangleMesh& mesh,
                             const BuildParams& buildParams)
: _mesh(mesh)
, _buildParams(buildParams)
, _buildStats(buildParams.collectStats)
{
  if (_mesh.GetTrianglesCount() > maxTrianglesCount) {
    throw KdTreeBuildingException(
        "Exceeded the maximum number of mesh triangles: " +
        std::to_string(maxTrianglesCount));
  }

  if (_buildParams.maxDepth <= 0)
    _buildParams.maxDepth = std::lround(
        8.0 + 1.3 * std::floor(std::log2(mesh.GetTrianglesCount())));

  _buildParams.maxDepth = std::min(_buildParams.maxDepth,
                                   static_cast<int>(KdTree::maxTraversalDepth));
}

KdTree KdTreeBuilder::buildTree()
{
  // initialize bounding boxes
  BoundingBox_f meshBounds;
  _triangleBounds.resize(_mesh.GetTrianglesCount());
  for (auto i = 0; i < _mesh.GetTrianglesCount(); ++i) {
    auto bounds = _mesh.GetTriangleBounds(i);
    _triangleBounds[i] = bounds;
    meshBounds = BoundingBox_f::Union(meshBounds, bounds);
  }

  // initialize working memory
  _edgesBuffer.resize(2 * _mesh.GetTrianglesCount());
  _trianglesBuffer.resize(_mesh.GetTrianglesCount() *
                          (_buildParams.maxDepth + 1));

  // fill triangle indices for root node
  for (int32_t i = 0; i < _mesh.GetTrianglesCount(); ++i)
    _trianglesBuffer[i] = i;

  // recursively build all nodes
  buildNode(meshBounds, _trianglesBuffer.data(), _mesh.GetTrianglesCount(),
            _buildParams.maxDepth, _trianglesBuffer.data(),
            _trianglesBuffer.data() + _mesh.GetTrianglesCount());

  _buildStats.finalizeStats(_nodes.size(), _triangleIndices.size());

  KdTree tree(std::move(_nodes), std::move(_triangleIndices), _mesh);
  return tree;
}

void KdTreeBuilder::buildNode(const BoundingBox_f& nodeBounds,
                              const int32_t* nodeTriangles,
                              int32_t nodeTrianglesCount, int depth,
                              int32_t* triangles0, int32_t* triangles1)
{
  if (_nodes.size() >= KdTree::Node::maxNodesCount)
    throw KdTreeBuildingException(
        "The maximum number of KdTree nodes has been reached: " +
        std::to_string(KdTree::Node::maxNodesCount));

  _buildStats.updateTrianglesStack(nodeTrianglesCount);

  struct StatsUpdater {
    StatsUpdater(BuildStats& stats, int32_t nodeTrianglesCount)
    : stats(stats)
    {
      stats.updateTrianglesStack(nodeTrianglesCount);
    }
    ~StatsUpdater()
    {
      stats.updateTrianglesStack(-1);
    }
    BuildStats& stats;
  } statsUpdater(_buildStats, nodeTrianglesCount);

  // check if leaf node should be created
  if (nodeTrianglesCount <= _buildParams.leafCandidateTrianglesCount ||
      depth == 0) {
    createLeaf(nodeTriangles, nodeTrianglesCount);
    _buildStats.newLeaf(nodeTrianglesCount, _buildParams.maxDepth - depth);
    return;
  }

  // select split position
  auto split = selectSplit(nodeBounds, nodeTriangles, nodeTrianglesCount);
  if (split.edge == -1) {
    createLeaf(nodeTriangles, nodeTrianglesCount);
    _buildStats.newLeaf(nodeTrianglesCount, _buildParams.maxDepth - depth);
    return;
  }
  float splitPosition = _edgesBuffer[split.edge].positionOnAxis;

  // classify triangles with respect to split
  int32_t n0 = 0;
  for (int32_t i = 0; i < split.edge; ++i) {
    if (_edgesBuffer[i].isStart()) {
      triangles0[n0++] = _edgesBuffer[i].triangleAndEndFlag;
    }
  }

  int32_t n1 = 0;
  for (int32_t i = split.edge + 1; i < 2 * nodeTrianglesCount; ++i) {
    if (_edgesBuffer[i].isEnd()) {
      int32_t triangle =
          _edgesBuffer[i].triangleAndEndFlag & BoundEdge::triangleMask;
      triangles1[n1++] = triangle;
    }
  }

  // add interior node and recursively create children nodes
  auto thisNodeIndex = static_cast<int32_t>(_nodes.size());
  _nodes.push_back(KdTree::Node());

  BoundingBox_f bounds0 = nodeBounds;
  bounds0.maxPoint[split.axis] = splitPosition;
  buildNode(bounds0, triangles0, n0, depth - 1, triangles0, triangles1 + n1);

  auto aboveChild = static_cast<int32_t>(_nodes.size());
  _nodes[thisNodeIndex].InitInteriorNode(split.axis, aboveChild, splitPosition);

  BoundingBox_f bound1 = nodeBounds;
  bound1.minPoint[split.axis] = splitPosition;
  buildNode(bound1, triangles1, n1, depth - 1, triangles0, triangles1);
}

void KdTreeBuilder::createLeaf(const int32_t* nodeTriangles,
                               int32_t nodeTrianglesCount)
{
  KdTree::Node node;
  if (nodeTrianglesCount == 0) {
    node.InitEmptyLeaf();
  }
  else if (nodeTrianglesCount == 1) {
    node.InitLeafWithSingleTriangle(nodeTriangles[0]);
  }
  else {
    node.InitLeafWithMultipleTriangles(
        nodeTrianglesCount, static_cast<int32_t>(_triangleIndices.size()));
    _triangleIndices.insert(_triangleIndices.end(), nodeTriangles,
                            nodeTriangles + nodeTrianglesCount);
  }
  _nodes.push_back(node);
}

KdTreeBuilder::Split KdTreeBuilder::selectSplit(const BoundingBox_f& nodeBounds,
                                                const int32_t* nodeTriangles,
                                                int32_t nodeTrianglesCount)
{
  // Determine axes iteration order.
  int axes[3];
  if (_buildParams.splitAlongTheLongestAxis) {
    Vector_f diag = nodeBounds.maxPoint - nodeBounds.minPoint;
    if (diag.x >= diag.y && diag.x >= diag.z) {
      axes[0] = 0;
      axes[1] = diag.y >= diag.z ? 1 : 2;
    }
    else if (diag.y >= diag.x && diag.y >= diag.z) {
      axes[0] = 1;
      axes[1] = diag.x >= diag.z ? 0 : 2;
    }
    else {
      axes[0] = 2;
      axes[1] = diag.x >= diag.y ? 0 : 1;
    }
    axes[2] = 3 - axes[0] - axes[1]; // since 0 + 1 + 2 == 3
  }
  else {
    axes[0] = 0;
    axes[1] = 1;
    axes[2] = 2;
  }

  // Select spliting axis and position. If
  // buildParams.splitAlongTheLongestAxis is true
  // then we stop at the first axis that gives a valid split.
  Split bestSplit = {-1, -1, std::numeric_limits<float>::infinity()};
  for (int index = 0; index < 3; ++index) {
    int axis = axes[index];

    // initialize edges
    for (int32_t i = 0; i < nodeTrianglesCount; ++i) {
      int32_t triangle = nodeTriangles[i];
      _edgesBuffer[2 * i + 0] = {_triangleBounds[triangle].minPoint[axis],
                                 static_cast<uint32_t>(triangle)};
      _edgesBuffer[2 * i + 1] = {_triangleBounds[triangle].maxPoint[axis],
                                 triangle | BoundEdge::endMask};
    }
    std::stable_sort(_edgesBuffer.data(),
                     _edgesBuffer.data() + 2 * nodeTrianglesCount,
                     BoundEdge::less);

    // select split position
    auto split = selectSplitForAxis(nodeBounds, nodeTrianglesCount, axis);
    if (split.edge != -1) {
      if (_buildParams.splitAlongTheLongestAxis)
        return split;
      if (split.cost < bestSplit.cost)
        bestSplit = split;
    }
  }

  // If split axis is not the last axis (2) then we should reinitialize
  // edgesBuffer to
  // contain data for split axis since edgesBuffer will be used later.
  if (bestSplit.axis == 0 || bestSplit.axis == 1) {
    for (int32_t i = 0; i < nodeTrianglesCount; ++i) {
      int triangle = nodeTriangles[i];
      _edgesBuffer[2 * i + 0] = {
          _triangleBounds[triangle].minPoint[bestSplit.axis],
          static_cast<uint32_t>(triangle)};
      _edgesBuffer[2 * i + 1] = {
          _triangleBounds[triangle].maxPoint[bestSplit.axis],
          triangle | BoundEdge::endMask};
    }
    std::stable_sort(_edgesBuffer.data(),
                     _edgesBuffer.data() + 2 * nodeTrianglesCount,
                     BoundEdge::less);
  }
  return bestSplit;
}

KdTreeBuilder::Split KdTreeBuilder::selectSplitForAxis(
    const BoundingBox_f& nodeBounds, int32_t nodeTrianglesCount, int axis) const
{
  static const int otherAxis[3][2] = {{1, 2}, {0, 2}, {0, 1}};
  const int otherAxis0 = otherAxis[axis][0];
  const int otherAxis1 = otherAxis[axis][1];
  const Vector_f diag = nodeBounds.maxPoint - nodeBounds.minPoint;

  const float s0 = 2.0f * (diag[otherAxis0] * diag[otherAxis1]);
  const float d0 = 2.0f * (diag[otherAxis0] + diag[otherAxis1]);
  const float invTotalS =
      1.0f / (2.0f * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z));
  const int32_t numEdges = 2 * nodeTrianglesCount;

  Split bestSplit = {-1, axis,
                     _buildParams.intersectionCost * nodeTrianglesCount};

  int32_t numBelow = 0;
  int32_t numAbove = nodeTrianglesCount;

  int32_t i = 0;
  while (i < numEdges) {
    BoundEdge edge = _edgesBuffer[i];

    // find group of edges with the same axis position: [i, groupEnd)
    int groupEnd = i + 1;
    while (groupEnd < numEdges &&
           edge.positionOnAxis == _edgesBuffer[groupEnd].positionOnAxis)
      ++groupEnd;

    // [i, middleEdge) - edges End points.
    // [middleEdge, groupEnd) - edges Start points.
    int middleEdge = i;
    while (middleEdge != groupEnd && _edgesBuffer[middleEdge].isEnd())
      ++middleEdge;

    numAbove -= middleEdge - i;

    float t = edge.positionOnAxis;
    if (t > nodeBounds.minPoint[axis] && t < nodeBounds.maxPoint[axis]) {
      auto belowS = s0 + d0 * (t - nodeBounds.minPoint[axis]);
      auto aboveS = s0 + d0 * (nodeBounds.maxPoint[axis] - t);

      auto pBelow = belowS * invTotalS;
      auto pAbove = aboveS * invTotalS;

      auto emptyBonus =
          (numBelow == 0 || numAbove == 0) ? _buildParams.emptyBonus : 0.0f;

      auto cost = _buildParams.traversalCost +
                  (1.0f - emptyBonus) * _buildParams.intersectionCost *
                      (pBelow * numBelow + pAbove * numAbove);

      if (cost < bestSplit.cost) {
        bestSplit.edge = (middleEdge == groupEnd) ? middleEdge - 1 : middleEdge;
        bestSplit.cost = cost;
      }
    }

    numBelow += groupEnd - middleEdge;
    i = groupEnd;
  }
  return bestSplit;
}

const KdTreeBuilder::BuildStats& KdTreeBuilder::getBuildStats() const
{
  return _buildStats;
}

KdTreeBuilder::BuildStats::BuildStats(bool enabled)
{
  _enabled = enabled;
}

void KdTreeBuilder::BuildStats::updateTrianglesStack(int nodeTrianglesCount)
{
  if (!_enabled)
    return;

  if (nodeTrianglesCount >= 0)
    _trianglesStack.push_back(nodeTrianglesCount);
  else
    _trianglesStack.pop_back();
}

void KdTreeBuilder::BuildStats::newLeaf(int leafTriangles, int depth)
{
  if (!_enabled)
    return;

  ++leafCount;

  if (leafTriangles == 0) {
    ++emptyLeafCount;
  }
  else // not empty leaf
  {
    _leafDepthAccumulated += depth;
    _leafDepthAppender.push_back(static_cast<uint8_t>(depth));
    _trianglesPerLeafAccumulated += leafTriangles;
  }
}

void KdTreeBuilder::BuildStats::finalizeStats(int32_t nodesCount,
                                              int32_t triangleIndicesCount)
{
  if (!_enabled)
    return;

  this->nodesCount = nodesCount;
  this->triangleIndicesCount = triangleIndicesCount;

  auto notEmptyLeafCount = leafCount - emptyLeafCount;

  trianglesPerLeaf =
      static_cast<double>(_trianglesPerLeafAccumulated) / notEmptyLeafCount;

  perfectDepth = static_cast<int>(ceil(log2(leafCount)));
  averageDepth = static_cast<double>(_leafDepthAccumulated) / notEmptyLeafCount;

  double accum = 0.0;
  for (auto depth : _leafDepthAppender) {
    auto diff = depth - averageDepth;
    accum += diff * diff;
  }
  depthStandardDeviation = sqrt(accum / notEmptyLeafCount);
}
