package main

import (
	"bufio"
	"common"
	"encoding/binary"
	"math"
	"os"
	"unsafe"
)

const (
	maxTraversalDepth        = 64
	maxNodesCount            = 0x40000000
	leafNodeFlags     uint32 = 3
)

type node [2]uint32

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

func (n *node) initLeafWithMultipleTriangles(numTriangles int32,
	triangleIndicesOffset int32) {
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

func (n node) trianglesCount() int32 {
	return int32(n[0] >> 2)
}

func (n node) index() int32 {
	return int32(n[1])
}

func (n node) splitAxis() int {
	return int(n[0] & leafNodeFlags)
}

func (n node) splitPosition() float32 {
	return *(*float32)(unsafe.Pointer(&n[1]))
}

func (n node) aboveChild() int32 {
	return int32(n[0] >> 2)
}

func (n *node) nextNode() *node {
	return (*node)(unsafe.Pointer(uintptr(unsafe.Pointer(n)) + 8))
}

type KdTree struct {
	nodes           []node
	triangleIndices []int32
	mesh            *TriangleMesh
	meshBounds      BBox64
}

type KdTreeIntersection struct {
	t       float64
	epsilon float64
}

func NewKdTree(fileName string, mesh *TriangleMesh) *KdTree {
	file, err := os.Open(fileName)
	common.Check(err)
	defer file.Close()

	reader := bufio.NewReader(file)

	var nodesCount int32
	err = binary.Read(reader, binary.LittleEndian, &nodesCount)
	common.Check(err)

	nodes := make([]node, nodesCount)
	err = binary.Read(reader, binary.LittleEndian, &nodes)
	common.Check(err)

	var triangleIndicesCount int32
	err = binary.Read(reader, binary.LittleEndian, &triangleIndicesCount)
	common.Check(err)

	triangleIndices := make([]int32, triangleIndicesCount)
	err = binary.Read(reader, binary.LittleEndian, &triangleIndices)
	common.Check(err)

	return &KdTree{
		nodes:           nodes,
		triangleIndices: triangleIndices,
		mesh:            mesh,
		meshBounds:      NewBBox64FromBBox32(mesh.GetBounds()),
	}
}

func (kdTree *KdTree) SaveToFile(fileName string) {
	file, err := os.Create(fileName)
	common.Check(err)
	defer file.Close()

	writer := bufio.NewWriter(file)

	nodesCount := int32(len(kdTree.nodes))
	err = binary.Write(writer, binary.LittleEndian, nodesCount)
	common.Check(err)

	err = binary.Write(writer, binary.LittleEndian, kdTree.nodes)
	common.Check(err)

	triangleIndicesCount := int32(len(kdTree.triangleIndices))
	err = binary.Write(writer, binary.LittleEndian, triangleIndicesCount)
	common.Check(err)

	err = binary.Write(writer, binary.LittleEndian, kdTree.triangleIndices)
	common.Check(err)

	err = writer.Flush()
	common.Check(err)
}

func (kdTree *KdTree) Intersect(ray *Ray) (bool, KdTreeIntersection) {
	tMin, tMax, intersectBounds := kdTree.meshBounds.Intersect(ray)
	if !intersectBounds {
		return false, KdTreeIntersection{t: math.Inf(+1)}
	}

	type traversalInfo struct {
		n    *node
		tMin float64
		tMax float64
	}

	var traversalStack [maxTraversalDepth]traversalInfo
	traversalStackSize := 0

	n := &kdTree.nodes[0]
	closestIntersection := TriangleIntersection{t: math.Inf(+1)}

	for closestIntersection.t > tMin {
		if n.isInteriorNode() {
			axis := n.splitAxis()

			distanceToSplitPlane := float64(n.splitPosition()) - ray.GetOrigin()[axis]

			belowChild := n.nextNode()
			aboveChild := &kdTree.nodes[n.aboveChild()]

			if distanceToSplitPlane != 0.0 { // general case
				var firstChild, secondChild *node

				if distanceToSplitPlane > 0 {
					firstChild = belowChild
					secondChild = aboveChild
				} else {
					firstChild = aboveChild
					secondChild = belowChild
				}

				// tSplit != 0 (since distanceToSplitPlane != 0)
				tSplit := distanceToSplitPlane * ray.GetInvDirection()[axis]
				if tSplit >= tMax || tSplit < 0 {
					n = firstChild
				} else if tSplit <= tMin {
					n = secondChild
				} else { // tMin < tSplit < tMax
					traversalStack[traversalStackSize] =
						traversalInfo{secondChild, tSplit, tMax}
					traversalStackSize++

					n = firstChild
					tMax = tSplit
				}
			} else { // special case, distanceToSplitPlane == 0.0
				if ray.GetDirection()[axis] > 0.0 {
					if tMin > 0.0 {
						n = aboveChild
					} else { // tMin == 0.0
						traversalStack[traversalStackSize] =
							traversalInfo{aboveChild, 0.0, tMax}
						traversalStackSize++

						// check single point [0.0, 0.0]
						n = belowChild
						tMax = 0.0
					}
				} else if ray.GetDirection()[axis] < 0.0 {
					if tMin > 0.0 {
						n = belowChild
					} else { // tMin == 0.0
						traversalStack[traversalStackSize] =
							traversalInfo{belowChild, 0.0, tMax}
						traversalStackSize++

						// check single point [0.0, 0.0]
						n = aboveChild
						tMax = 0.0
					}
				} else { // ray.direction[axis] == 0.0
					// for both nodes check [tMin, tMax] range
					traversalStack[traversalStackSize] =
						traversalInfo{aboveChild, tMin, tMax}
					traversalStackSize++

					n = belowChild
				}
			}
		} else { // leaf node
			kdTree.IntersectLeafTriangles(ray, *n, &closestIntersection)

			if traversalStackSize == 0 {
				break
			}

			traversalStackSize--
			n = traversalStack[traversalStackSize].n
			tMin = traversalStack[traversalStackSize].tMin
			tMax = traversalStack[traversalStackSize].tMax
		}
	}

	if closestIntersection.t == math.Inf(+1) {
		return false, KdTreeIntersection{t: math.Inf(+1)}
	}

	return true,
		KdTreeIntersection{
			t:       closestIntersection.t,
			epsilon: closestIntersection.epsilon,
		}
}

func (kdTree *KdTree) IntersectLeafTriangles(ray *Ray, leaf node,
	closestIntersection *TriangleIntersection) {

	vertices := kdTree.mesh.vertices
	triangles := kdTree.mesh.triangles

	if leaf.trianglesCount() == 1 {
		triangleIndex := leaf.index()
		indices := triangles[triangleIndex]
		triangle := Triangle{[3]Vector64{
			NewVector64FromVector32(vertices[indices[0]]),
			NewVector64FromVector32(vertices[indices[1]]),
			NewVector64FromVector32(vertices[indices[2]]),
		}}
		hitFound, triangleIntersection := IntersectTriangle(ray, &triangle)
		if hitFound && triangleIntersection.t < closestIntersection.t {
			*closestIntersection = triangleIntersection
		}
	} else {
		for i := int32(0); i < leaf.trianglesCount(); i++ {
			triangleIndex := kdTree.triangleIndices[leaf.index()+i]
			indices := triangles[triangleIndex]
			triangle := Triangle{[3]Vector64{
				NewVector64FromVector32(vertices[indices[0]]),
				NewVector64FromVector32(vertices[indices[1]]),
				NewVector64FromVector32(vertices[indices[2]]),
			}}
			hitFound, triangleIntersection := IntersectTriangle(ray, &triangle)
			if hitFound && triangleIntersection.t < closestIntersection.t {
				*closestIntersection = triangleIntersection
			}
		}
	}
}

func (kdTree *KdTree) GetHash() uint64 {
	var hash uint64
	for _, node := range kdTree.nodes {
		hash = common.CombineHashes(hash, uint64(node[0]))
		hash = common.CombineHashes(hash, uint64(node[1]))
	}
	for _, index := range kdTree.triangleIndices {
		hash = common.CombineHashes(hash, uint64(uint32(index)))
	}
	return hash
}
