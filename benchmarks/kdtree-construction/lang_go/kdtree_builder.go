package main

import (
	"fmt"
	"math"
	"sort"
)

type BuildParams struct {
	IntersectionCost            float32
	TraversalCost               float32
	EmptyBonus                  float32
	LeafCandidateTrianglesCount int
	MaxDepth                    int
	SplitAlongTheLongestAxis    bool
	CollectStats                bool
}

func NewBuildParams() BuildParams {
	return BuildParams{
		IntersectionCost:            80,
		TraversalCost:               1,
		EmptyBonus:                  0.3,
		LeafCandidateTrianglesCount: 2,
		MaxDepth:                    -1,
		SplitAlongTheLongestAxis:    false,
		CollectStats:                true,
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

func (stats *BuildStats) Print() {
	fmt.Println("LeafCount =", stats.LeafCount)
	fmt.Println("EmptyLeafCount =", stats.EmptyLeafCount)
	fmt.Println("TrianglesPerLeaf =", stats.TrianglesPerLeaf)
	fmt.Println("PerfectDepth =", stats.PerfectDepth)
	fmt.Println("AverageDepth =", stats.AverageDepth)
	fmt.Println("DepthStandardDeviation =", stats.DepthStandardDeviation)
	fmt.Println()
}

func (stats *BuildStats) updateTrianglesStack(nodeTrianglesCount int32) {
	if !stats.enabled {
		return
	}

	if nodeTrianglesCount >= 0 {
		stats.trianglesStack = append(stats.trianglesStack, nodeTrianglesCount)
	} else {
		stats.trianglesStack = stats.trianglesStack[0:len(stats.trianglesStack) - 1]	
	}
}

func (stats *BuildStats) newLeaf(leafTriangles int, depth int) {
	if !stats.enabled {
		return
	}

	stats.LeafCount++

	if leafTriangles == 0 {
		stats.EmptyLeafCount++
	} else { // not empty leaf
		stats.leafDepthAccumulated += int64(depth)
		stats.leafDepthValues = append(stats.leafDepthValues, uint8(depth))
		stats.trianglesPerLeafAccumulated += int64(leafTriangles)
	}
}

func (stats *BuildStats) finalizeStats() {
	if !stats.enabled {
		return
	}

	notEmptyLeafCount := stats.LeafCount - stats.EmptyLeafCount

	stats.TrianglesPerLeaf = float64(stats.trianglesPerLeafAccumulated) / float64(notEmptyLeafCount)
	stats.PerfectDepth = int32(math.Ceil(math.Log2(float64(stats.LeafCount))))
	stats.AverageDepth = float64(stats.leafDepthAccumulated) / float64(notEmptyLeafCount)

	accum := 0.0
	for _, depth := range stats.leafDepthValues {
		diff := float64(depth) - stats.AverageDepth
		accum += diff*diff
	}
	stats.DepthStandardDeviation = math.Sqrt(accum / float64(notEmptyLeafCount))
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

type boundEdgeSorter []boundEdge

func (s boundEdgeSorter) Len() int {
	return len(s)
}

func (s boundEdgeSorter) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func (s boundEdgeSorter) Less(i, j int) bool {
	if s[i].positionOnAxis == s[j].positionOnAxis {
		return s[i].isEnd() && s[j].isStart()
	} else {
		return s[i].positionOnAxis < s[j].positionOnAxis
	}
}

type KdTreeBuilder struct {
	mesh            *TriangleMesh
	buildParams     BuildParams
	buildStats      BuildStats
	triangleBounds  []BBox32
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
		buildParams.MaxDepth = int(math.Floor(0.5 + 8.0 + 1.3*trianglesCountLog))
	}
	if buildParams.MaxDepth > maxTraversalDepth {
		buildParams.MaxDepth = maxTraversalDepth
	}

	builder := &KdTreeBuilder{
		mesh:        mesh,
		buildParams: buildParams,
	}
	if buildParams.CollectStats {
		builder.buildStats.enabled = true
	}
	return builder, nil
}

func (builder *KdTreeBuilder) BuildKdTree() *KdTree {
	trianglesCount := builder.mesh.GetTrianglesCount()

	// initialize bounding boxes
	meshBounds := NewBBox32()
	builder.triangleBounds = make([]BBox32, trianglesCount)

	for i := int32(0); i < trianglesCount; i++ {
		builder.triangleBounds[i] = builder.mesh.GetTriangleBounds(i)
		meshBounds = BBox32Union(meshBounds, builder.triangleBounds[i])
	}

	// initialize working memory
	builder.edgesBuffer = make([]boundEdge, 2*trianglesCount)
	trianglesBufferSize := int(trianglesCount) * (builder.buildParams.MaxDepth + 1)
	builder.trianglesBuffer = make([]int32, trianglesBufferSize)

	// fill triangle indices for root node
	for i := int32(0); i < trianglesCount; i++ {
		builder.trianglesBuffer[i] = i
	}

	// recursively build all nodes
	builder.buildNode(meshBounds, builder.trianglesBuffer[0:trianglesCount],
		builder.buildParams.MaxDepth, 0, int(trianglesCount))

	builder.buildStats.finalizeStats()
	return &KdTree{builder.nodes, builder.triangleIndices, builder.mesh, meshBounds}
}

func (builder *KdTreeBuilder) buildNode(nodeBounds BBox32, nodeTriangles []int32, depth int,
	offset0 int, offset1 int) error {
	if len(builder.nodes) >= maxNodesCount {
		return fmt.Errorf("The maximum number of KdTree nodes has been reached: %d",
			maxNodesCount)
	}

	builder.buildStats.updateTrianglesStack(int32(len(nodeTriangles)))
	defer func() { builder.buildStats.updateTrianglesStack(-1) } ()

	// check if leaf node should be created
	if len(nodeTriangles) <= builder.buildParams.LeafCandidateTrianglesCount || depth == 0 {
		builder.createLeaf(nodeTriangles)
		builder.buildStats.newLeaf(len(nodeTriangles), builder.buildParams.MaxDepth - depth)
		return nil
	}

	// select split position
	split := builder.selectSplit(nodeBounds, nodeTriangles)
	if split.edge == -1 {
		builder.createLeaf(nodeTriangles)
		builder.buildStats.newLeaf(len(nodeTriangles), builder.buildParams.MaxDepth - depth)
		return nil
	}
	splitPosition := builder.edgesBuffer[split.edge].positionOnAxis

	// classify triangles with respect to split
	n0 := 0
	for i := 0; i < int(split.edge); i++ {
		if builder.edgesBuffer[i].isStart() {
			triangle := int32(builder.edgesBuffer[i].triangleAndEndFlag)
			builder.trianglesBuffer[offset0+n0] = triangle
			n0++
		}
	}

	n1 := 0
	for i := int(split.edge) + 1; i < 2*len(nodeTriangles); i++ {
		if builder.edgesBuffer[i].isEnd() {
			triangle := int32(builder.edgesBuffer[i].triangleAndEndFlag & edgeTriangleMask)
			builder.trianglesBuffer[offset1+n1] = triangle
			n1++
		}
	}

	// add interior node and recursively create children nodes
	thisNodeIndex := len(builder.nodes)
	builder.nodes = append(builder.nodes, node{})

	bounds0 := nodeBounds
	bounds0.maxPoint[split.axis] = splitPosition
	err := builder.buildNode(bounds0, builder.trianglesBuffer[0:n0], depth-1, 0, offset1+n1)
	if err != nil {
		return err
	}

	aboveChild := int32(len(builder.nodes))
	builder.nodes[thisNodeIndex].initInteriorNode(split.axis, aboveChild, splitPosition)

	bounds1 := nodeBounds
	bounds1.minPoint[split.axis] = splitPosition
	err = builder.buildNode(bounds1, builder.trianglesBuffer[offset1:offset1+n1], depth-1, 0, offset1)
	if err != nil {
		return err
	}
	return nil
}

func (builder *KdTreeBuilder) createLeaf(nodeTriangles []int32) {
	var theNode node
	if len(nodeTriangles) == 0 {
		theNode.initEmptyLeaf()
	} else if len(nodeTriangles) == 1 {
		theNode.initLeafWithSingleTriangle(nodeTriangles[0])
	} else {
		theNode.initLeafWithMultipleTriangles(len(nodeTriangles), int32(len(builder.triangleIndices)))
		builder.triangleIndices = append(builder.triangleIndices, nodeTriangles...)
	}
	builder.nodes = append(builder.nodes, theNode)
}

type split struct {
	edge int32
	axis int
	cost float32
}

func (builder *KdTreeBuilder) selectSplit(nodeBounds BBox32, nodeTriangles []int32) split {
	// Determine axes iteration order.
	var axes [3]int
	if builder.buildParams.SplitAlongTheLongestAxis {
		diag := VSub32(nodeBounds.maxPoint, nodeBounds.minPoint)
		if diag[0] >= diag[1] && diag[0] >= diag[2] {
			axes[0] = 0
			if diag[1] >= diag[2] {
				axes[1] = 1
			} else {
				axes[1] = 2
			}
		} else if diag[1] >= diag[0] && diag[1] >= diag[2] {
			axes[0] = 1
			if diag[0] >= diag[2] {
				axes[1] = 0
			} else {
				axes[1] = 2
			}
		} else {
			axes[0] = 2
			if diag[0] >= diag[1] {
				axes[1] = 0
			} else {
				axes[1] = 1
			}
		}
		axes[2] = 3 - axes[0] - axes[1]
	} else {
		axes = [3]int{0, 1, 2}
	}

	// Select spliting axis and position. If buildParams.SplitAlongTheLongestAxis is true
	// then we stop at the first axis that gives a valid split.
	bestSplit := split{-1, -1, float32(math.Inf(+1))}

	for _, axis := range axes {
		// initialize edges
		for i, triangle := range nodeTriangles {
			builder.edgesBuffer[2*i+0] =
				boundEdge{builder.triangleBounds[triangle].minPoint[axis], uint32(triangle)}
			builder.edgesBuffer[2*i+1] =
				boundEdge{builder.triangleBounds[triangle].maxPoint[axis], uint32(triangle) | edgeEndMask}
		}
		sort.Stable(boundEdgeSorter(builder.edgesBuffer[0 : len(nodeTriangles)*2]))

		// select split position
		currentSplit := builder.selectSplitForAxis(nodeBounds, int32(len(nodeTriangles)), axis)
		if currentSplit.edge != -1 {
			if builder.buildParams.SplitAlongTheLongestAxis {
				return currentSplit
			}
			if currentSplit.cost < bestSplit.cost {
				bestSplit = currentSplit
			}
		}
	}

	// If split axis is not the last axis (2) then we should reinitialize edgesBuffer to
	// contain data for split axis since edgesBuffer will be used later.
	if bestSplit.axis == 0 || bestSplit.axis == 1 {
		for i, triangle := range nodeTriangles {
			builder.edgesBuffer[2*i+0] =
				boundEdge{builder.triangleBounds[triangle].minPoint[bestSplit.axis], uint32(triangle)}
			builder.edgesBuffer[2*i+1] =
				boundEdge{builder.triangleBounds[triangle].maxPoint[bestSplit.axis], uint32(triangle) | edgeEndMask}
		}
		sort.Stable(boundEdgeSorter(builder.edgesBuffer[0 : len(nodeTriangles)*2]))
	}
	return bestSplit
}

var otherAxis = [3][2]int{{1, 2}, {0, 2}, {0, 1}}

func (builder *KdTreeBuilder) selectSplitForAxis(nodeBounds BBox32, nodeTrianglesCount int32, axis int) split {
	buildParams := &builder.buildParams

	otherAxis0 := otherAxis[axis][0]
	otherAxis1 := otherAxis[axis][1]
	diag := VSub32(nodeBounds.maxPoint, nodeBounds.minPoint)

	s0 := 2.0 * (diag[otherAxis0] * diag[otherAxis1])
	d0 := 2.0 * (diag[otherAxis0] + diag[otherAxis1])
	invTotalS := 1.0 / (2.0 * (diag[0]*diag[1] + diag[0]*diag[2] + diag[1]*diag[2]))
	numEdges := nodeTrianglesCount * 2

	bestSplit := split{-1, axis, buildParams.IntersectionCost * float32(nodeTrianglesCount)}

	numBelow := int32(0)
	numAbove := nodeTrianglesCount

	for i := int32(0); i < numEdges; {
		edge := builder.edgesBuffer[i]

		// find group of edges with the same axis position: [i, groupEnd)
		groupEnd := i + 1
		for groupEnd < numEdges && edge.positionOnAxis == builder.edgesBuffer[groupEnd].positionOnAxis {
			groupEnd++
		}

		// [i, middleEdge) - edges End points.
		// [middleEdge, groupEnd) - edges Start points.
		middleEdge := i
		for middleEdge != groupEnd && builder.edgesBuffer[middleEdge].isEnd() {
			middleEdge++
		}

		numAbove -= middleEdge - i

		t := edge.positionOnAxis
		if t > nodeBounds.minPoint[axis] && t < nodeBounds.maxPoint[axis] {
			belowS := s0 + d0*(t-nodeBounds.minPoint[axis])
			aboveS := s0 + d0*(nodeBounds.maxPoint[axis]-t)

			pBelow := belowS * invTotalS
			pAbove := aboveS * invTotalS

			emptyBonus := float32(0.0)
			if numBelow == 0 || numAbove == 0 {
				emptyBonus = buildParams.EmptyBonus
			}

			cost := buildParams.TraversalCost +
				(1.0-emptyBonus)*buildParams.IntersectionCost*(pBelow*float32(numBelow)+pAbove*float32(numAbove))

			if cost < bestSplit.cost {
				bestSplit.edge = middleEdge
				if middleEdge == groupEnd {
					bestSplit.edge -= 1
				}
				bestSplit.cost = cost
			}
		}

		numBelow += groupEnd - middleEdge
		i = groupEnd
	}
	return bestSplit
}
