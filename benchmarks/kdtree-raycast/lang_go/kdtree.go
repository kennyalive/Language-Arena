package main

import (
	"bufio"
	"encoding/binary"
	"os"
	"unsafe"
)

const maxTraversalDepth = 64

type node [2]uint32

const (
	maxNodesCount        = 0x40000000
	leafNodeFlags uint32 = 3
)

func (n *node) initInteriorNode(axis int, aboveChild int32, split float32) {
	n[0] = uint32(axis) | uint32(aboveChild)<<2
	n[1] = *(*uint32)(unsafe.Pointer(&split))
}

func (n *node) initEmptyLeaf() {
	n[0] = leafNodeFlags // = 3
}

func (n *node) initLeafWithSingleTriangle(triangleIndex int32) {
	n[0] = leafNodeFlags | 1<<2 // = 7
	n[1] = uint32(triangleIndex)
}

func (n *node) initLeafWithMultipleTriangles(numTriangles int, triangleIndicesOffset int32) {
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

func (n node) getLeafTrianglesCount() int {
	return int(n[0] >> 2)
}

func (n node) getInteriorNodeSplitAxis() int {
	return int(n[0] & leafNodeFlags)
}

func (n node) getInteriorNodeAboveChild() int32 {
	return int32(n[0] >> 2)
}

type KdTree struct {
	nodes           []node
	triangleIndices []int32
	mesh            *TriangleMesh
	meshBounds      BBox32
}

func NewKdTree(fileName string, mesh *TriangleMesh) *KdTree {
	file, err := os.Open(fileName)
	checkError(err)
	defer file.Close()

	reader := bufio.NewReader(file)

	var nodesCount int32
	err = binary.Read(reader, binary.LittleEndian, &nodesCount)
	checkError(err)

	nodes := make([]node, nodesCount)
	err = binary.Read(reader, binary.LittleEndian, &nodes)
	checkError(err)

	var triangleIndicesCount int32
	err = binary.Read(reader, binary.LittleEndian, &triangleIndicesCount)
	checkError(err)

	triangleIndices := make([]int32, triangleIndicesCount)
	err = binary.Read(reader, binary.LittleEndian, &triangleIndices)
	checkError(err)

	return &KdTree{
		nodes: nodes,
		triangleIndices: triangleIndices,
		mesh: mesh,
		meshBounds: mesh.GetBounds(),
	}
}

func (kdTree *KdTree) SaveToFile(fileName string) {
	file, err := os.Create(fileName)
	checkError(err)
	defer file.Close()

	writer := bufio.NewWriter(file)
	binary.Write(writer, binary.LittleEndian, int32(len(kdTree.nodes)))
	binary.Write(writer, binary.LittleEndian, kdTree.nodes)
	binary.Write(writer, binary.LittleEndian, int32(len(kdTree.triangleIndices)))
	binary.Write(writer, binary.LittleEndian, kdTree.triangleIndices)
	writer.Flush()
}

func (kdTree *KdTree) Intersect(ray *Ray) (intersection Intersection, found bool) {
	return 
}
