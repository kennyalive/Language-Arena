package main

import "math"

type Intersection struct {
	RayT float64
	RayTEpsilon float64
}

func NewIntersection() Intersection {
	return Intersection {
		RayT: math.Inf(+1),
	}
}
