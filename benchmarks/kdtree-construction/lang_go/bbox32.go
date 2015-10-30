package main

import (
	"math"
)

type BBox32 struct {
	minPoint Vector32
	maxPoint Vector32
}

func NewBBox32() BBox32 {
	posInf := float32(math.Inf(+1))
	negInf := float32(math.Inf(-1))

	return BBox32{
		minPoint: Vector32{posInf, posInf, posInf},
		maxPoint: Vector32{negInf, negInf, negInf},
	}
}

func NewBBox32FromPoints(minPoint Vector32, maxPoint Vector32) BBox32 {
	return BBox32{
		minPoint: minPoint,
		maxPoint: maxPoint,
	}
}

func NewBBox32FromPoint(point Vector32) BBox32 {
	return BBox32{
		minPoint: point,
		maxPoint: point,
	}
}

func f32Min(a, b float32) float32 {
	if a < b {
		return a
	} else {
		return b
	}
}

func f32Max(a, b float32) float32 {
	if a > b {
		return a
	} else {
		return b
	}
}

func (bbox BBox32) Extend(point Vector32) {
	bbox.minPoint[0] = f32Min(bbox.minPoint[0], point[0])
	bbox.minPoint[1] = f32Min(bbox.minPoint[1], point[1])
	bbox.minPoint[2] = f32Min(bbox.minPoint[2], point[2])

	bbox.maxPoint[0] = f32Max(bbox.maxPoint[0], point[0])
	bbox.maxPoint[1] = f32Max(bbox.maxPoint[1], point[1])
	bbox.maxPoint[2] = f32Max(bbox.maxPoint[2], point[2])
}

func BBox32Union(bbox BBox32, bbox2 BBox32) BBox32 {
	return BBox32{
		minPoint: Vector32{
			f32Min(bbox.minPoint[0], bbox2.minPoint[0]),
			f32Min(bbox.minPoint[1], bbox2.minPoint[1]),
			f32Min(bbox.minPoint[2], bbox2.minPoint[2]),
		},
		maxPoint: Vector32{
			f32Max(bbox.maxPoint[0], bbox2.maxPoint[0]),
			f32Max(bbox.maxPoint[1], bbox2.maxPoint[1]),
			f32Max(bbox.maxPoint[2], bbox2.maxPoint[2]),
		},
	}
}
