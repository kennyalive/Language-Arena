package main

import (
	"math"
)

type BBox64 struct {
	minPoint Vector64
	maxPoint Vector64
}

func NewBBox64() BBox64 {
	posInf := math.Inf(+1)
	negInf := math.Inf(-1)

	return BBox64{
		minPoint: Vector64{posInf, posInf, posInf},
		maxPoint: Vector64{negInf, negInf, negInf},
	}
}

func BBox64FromPoints(minPoint Vector64, maxPoint Vector64) BBox64 {
	return BBox64{
		minPoint: minPoint,
		maxPoint: maxPoint,
	}
}

func BBox64FromPoint(point Vector64) BBox64 {
	return BBox64{
		minPoint: point,
		maxPoint: point,
	}
}

func (bbox *BBox64) Extend(point Vector64) {
	bbox.minPoint[0] = math.Min(bbox.minPoint[0], point[0])
	bbox.minPoint[1] = math.Min(bbox.minPoint[1], point[1])
	bbox.minPoint[2] = math.Min(bbox.minPoint[2], point[2])

	bbox.maxPoint[0] = math.Max(bbox.maxPoint[0], point[0])
	bbox.maxPoint[1] = math.Max(bbox.maxPoint[1], point[1])
	bbox.maxPoint[2] = math.Max(bbox.maxPoint[2], point[2])
}

func BBox64Union(bbox BBox64, bbox2 BBox64) BBox64 {
	return BBox64{
		minPoint: Vector64{
			math.Min(bbox.minPoint[0], bbox2.minPoint[0]),
			math.Min(bbox.minPoint[1], bbox2.minPoint[1]),
			math.Min(bbox.minPoint[2], bbox2.minPoint[2]),
		},
		maxPoint: Vector64{
			math.Max(bbox.maxPoint[0], bbox2.maxPoint[0]),
			math.Max(bbox.maxPoint[1], bbox2.maxPoint[1]),
			math.Max(bbox.maxPoint[2], bbox2.maxPoint[2]),
		},
	}
}
