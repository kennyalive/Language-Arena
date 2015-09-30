package main

import (
	"fmt"
	"math"
)

type BuildParams struct {
	IntersectionCost           float32
	TraversalCost              float32
	EmptyBonus                 float32
	LeafCandidateTriangleCount uint32
	MaxDepth                   int32
	SplitAlongTheLongestAxis   bool
	CollectStats               bool
}

func NewBuildParams() BuildParams {
	return BuildParams{
		IntersectionCost:           80,
		TraversalCost:              1,
		EmptyBonus:                 0.5,
		LeafCandidateTriangleCount: 2,
		MaxDepth:                   -1,
		SplitAlongTheLongestAxis:   false,
		CollectStats:               false,
	}
}

type BuildStats struct {
	LeafCount                   int32
	EmptyLeafCount              int32
	TrianglesPerLeaf            float64
	PerfectDepth                int32
	AverageDepth                float64
	DepthStandardDeviation      float64
	enabled                     bool
	trianglesPerLeafAccumulated int64
	leafDepthAccumulated        int64
	leafDepthValues             []uint8
	trianglesStack              []int32
}

const (
	edgeEndMask      uint32 = 0x80000000
	edgeTriangleMask uint32 = 0x7fffffff
)

type boundEdge struct {
	positionOnAxis     float32
	triangleAndEndFlag uint32
}

func (e boundEdge) isStart() bool {
	return e.triangleAndEndFlag&edgeEndMask == 0
}

func (e boundEdge) isEnd() bool {
	return !e.isStart()
}

type KdTreeBuilder struct {
	mesh            *TriangleMesh
	buildParams     BuildParams
	buildStats      *BuildStats
	triangleBounds  []BoundingBox
	edgesBuffer     []boundEdge
	trianglesBuffer []int32
	nodes           []node
	triangleIndices []int32
}

func NewKdTreeBuilder(mesh *TriangleMesh, buildParams BuildParams) (*KdTreeBuilder, error) {
	// max count is chosen such that maxTrianglesCount * 2 is still
	// an int32, this simplifies implementation.
	const maxTrianglesCount = 0x3fffffff // max ~ 1 billion triangles

	if mesh.GetTrianglesCount() > maxTrianglesCount {
		return nil, fmt.Errorf("Exceeded the maximum number of mesh triangles: %d",
			maxTrianglesCount)
	}

	if buildParams.MaxDepth <= 0 {
		trianglesCountLog := math.Floor(math.Log2(float64(mesh.GetTrianglesCount())))
		buildParams.MaxDepth = int32(math.Floor(0.5 + 8.0 + 1.3*trianglesCountLog))
	}

	if buildParams.MaxDepth > maxTraversalDepth {
		buildParams.MaxDepth = maxTraversalDepth
	}

	var buildStats *BuildStats
	if buildParams.CollectStats {
		buildStats = new(BuildStats)
	}

	return &KdTreeBuilder{
		mesh:        mesh,
		buildParams: buildParams,
		buildStats:  buildStats,
	}, nil
}

func (builder *KdTreeBuilder) BuildKdTree() *KdTree {
	return nil
}
