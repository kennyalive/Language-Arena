package main

import (
	"bufio"
	"encoding/binary"
	"math"
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

func (n node) getLeafTriangleIndex() int {
	return int(n[1])
}

func (n node) getLeafTriangleIndicesOffset() int {
	return int(n[1])
}

func (n node) getInteriorNodeSplitAxis() int {
	return int(n[0] & leafNodeFlags)
}

func (n node) getInteriorNodeAboveChild() int32 {
	return int32(n[0] >> 2)
}

func (n node) getInteriorNodeSplit() float32 {
	return *(*float32)(unsafe.Pointer(&n[1]))
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

func (kdTree *KdTree) Intersect(ray *Ray) (intersection Intersection, hitFound bool) {
	meshBounds64 := BBox64FromBBox32(kdTree.meshBounds)
	tMin, tMax, intersectBounds := meshBounds64.Intersect(ray)
	if !intersectBounds {
		return
	}

	type traversalInfo struct {
		theNode *node
		tMin float64
		tMax float64
	}

	var traversalStack [maxTraversalDepth]traversalInfo
	traversalStackSize := 0

	theNode := &kdTree.nodes[0]

	var closestIntersection TriangleIntersection
	closestIntersection.t = math.Inf(+1)

	advanceNodePointer := func(n *node) (*node) {
		return (*node)(unsafe.Pointer(uintptr(unsafe.Pointer(n)) + 8))
	}

	for closestIntersection.t > tMin {
		if theNode.isInteriorNode() {
			axis := theNode.getInteriorNodeSplitAxis()
			distanceToSplitPlane :=float64(theNode.getInteriorNodeSplit()) - ray.GetOrigin()[axis]

			if distanceToSplitPlane == 0 {
				if ray.GetDirection()[axis] > 0 {
					theNode = &kdTree.nodes[theNode.getInteriorNodeAboveChild()]
				} else {
					theNode = advanceNodePointer(theNode)
				}
			} else {
				var firstChild, secondChild *node
				if distanceToSplitPlane > 0 {
					firstChild = advanceNodePointer(theNode)
					secondChild = &kdTree.nodes[theNode.getInteriorNodeAboveChild()]
				} else {
					firstChild = &kdTree.nodes[theNode.getInteriorNodeAboveChild()]
					secondChild = advanceNodePointer(theNode)
				}

				tSplit := distanceToSplitPlane * ray.GetInvDirection()[axis] // != 0
				if tSplit >= tMax || tSplit < 0 {
					theNode = firstChild
				} else if tSplit <= tMin {
					theNode = secondChild
				} else { // tMin < tSplit < tMax
					traversalStack[traversalStackSize] = traversalInfo{secondChild, tSplit, tMax}
					traversalStackSize++
					theNode = firstChild
					tMax = tSplit
				}
			}
		} else { // leaf node
			trianglesCount := theNode.getLeafTrianglesCount()
			if trianglesCount == 1 {
				triangleIndex := theNode.getLeafTriangleIndex()
				indices := kdTree.mesh.triangles[triangleIndex]
				triangle := Triangle{
					Vector64From32(kdTree.mesh.vertices[indices[0]]),
					Vector64From32(kdTree.mesh.vertices[indices[1]]),
					Vector64From32(kdTree.mesh.vertices[indices[2]]),
				}
				triangleIntersection, hitFound := IntersectTriangle(ray, &triangle)
				if hitFound && triangleIntersection.t < closestIntersection.t {
					closestIntersection = triangleIntersection
				}
			} else {
				for i:=0; i < trianglesCount; i++ {
					triangleIndex := kdTree.triangleIndices[theNode.getLeafTriangleIndicesOffset() + i]
					indices := kdTree.mesh.triangles[triangleIndex]
					triangle := Triangle{
						Vector64From32(kdTree.mesh.vertices[indices[0]]),
						Vector64From32(kdTree.mesh.vertices[indices[1]]),
						Vector64From32(kdTree.mesh.vertices[indices[2]]),
					}
					triangleIntersection, hitFound := IntersectTriangle(ray, &triangle)
					if hitFound && triangleIntersection.t < closestIntersection.t {
						closestIntersection = triangleIntersection
					}
				}
			}

			if traversalStackSize == 0 {
				break
			}

			traversalStackSize--
			theNode = traversalStack[traversalStackSize].theNode
			tMin = traversalStack[traversalStackSize].tMin
			tMax = traversalStack[traversalStackSize].tMax
		}
	}

	hitFound = closestIntersection.t < math.Inf(+1)
	if hitFound {
		intersection.RayT = closestIntersection.t
		intersection.RayTEpsilon = closestIntersection.rayEpsilon
	}
	return
}
