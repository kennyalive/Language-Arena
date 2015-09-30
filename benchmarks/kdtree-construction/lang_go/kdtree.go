package main

import (
	"unsafe"
)

type KdTree struct {
	nodes           []node
	triangleIndices []int32
}

const maxTraversalDepth = 64

type node [2]uint32

const (
	maxNodesCount int32  = 0x40000000
	leafNodeFlags uint32 = 3
)

func (n node) initInteriorNode(axis uint32, aboveChild int32, split float32) {
	n[0] = axis | uint32(aboveChild)<<2
	n[1] = *(*uint32)(unsafe.Pointer(&split))
}

func (n node) initEmptyLeaf() {
	n[0] = leafNodeFlags // = 3
}

func (n node) initLeafWithSingleTriangle(triangleIndex int32) {
	n[0] = leafNodeFlags | 1<<2 // = 7
}

func (n node) initLeafWithMultipleTriangles(numTriangles int32, triangleIndicesOffset int32) {
	// n[0] = 11, 15, 19, ... (for numTriangles = 2, 3, 4, ...)
	n[0] = leafNodeFlags | uint32(numTriangles)<<2
	n[1] = uint32(triangleIndicesOffset)
}

func (n node) isLeaf() bool {
	return n[0]&leafNodeFlags == leafNodeFlags
}

func (n node) isInteriorNode() bool {
	return !n.isLeaf()
}

func (n node) getLeafTrianglesCount() int32 {
	return int32(n[0] >> 2)
}

func (n node) getInteriorNodeSplitAxis() uint32 {
	return n[0] & leafNodeFlags
}

func (n node) getInteriorNodeAboveChild() int32 {
	return int32(n[0] >> 2)
}
