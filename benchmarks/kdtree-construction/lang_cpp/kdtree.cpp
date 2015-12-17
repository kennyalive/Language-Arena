#include "kdtree.h"

KdTree::KdTree(KdTree&& other)
    : _nodes(std::move(other._nodes)),
      _triangleIndices(std::move(other._triangleIndices))
{
}
