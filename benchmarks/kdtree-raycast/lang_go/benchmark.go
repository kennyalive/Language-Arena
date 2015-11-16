package main

import (
	"fmt"
	"math/rand"
	"time"
)

func benchmarkKdTree(kdTree *KdTree) {
	// compute volume for generating test rays
	diagonal := Vector64From32(VSub32(kdTree.meshBounds.maxPoint, kdTree.meshBounds.minPoint))
	delta := 2.0 * VLength64(diagonal)

	bounds := BBox64FromPoints(
		VSub64(Vector64From32(kdTree.meshBounds.minPoint), Vector64FromScalar(delta)),
		VAdd64(Vector64From32(kdTree.meshBounds.maxPoint), Vector64FromScalar(delta)),
	)

	var lastHit Vector64
	var lastEpsilonOffset float64

	const benchmarkRays = 10000000
	raysTested := 0
	
	randForRange:= func(a, b float64) float64 {
		return a + (b - a)*rand.Float64()
	}

	start := time.Now()
	for ;raysTested < benchmarkRays; raysTested++ {
		// generate ray origin
		origin := Vector64{
			randForRange(bounds.minPoint[0], bounds.maxPoint[0]),
			randForRange(bounds.minPoint[1], bounds.maxPoint[1]),
			randForRange(bounds.minPoint[2], bounds.maxPoint[2]),
		}

		if rand.Float64() < 0.25 {
			origin = lastHit
		}

		// generate ray direction
		direction := UniformSampleSphere(rand.Float64(), rand.Float64())

		if rand.Float64() < 1.0/32.0 {
			direction[0] = 0
			direction[1] = 0
		} else if rand.Float64() < 1.0/32.0 {
			direction[0] = 0
			direction[2] = 0
		} else if rand.Float64() < 1.0/32.0 {
			direction[1] = 0
			direction[2] = 0
		}
		direction = VNormalized64(direction)

		// initialize ray
		ray := RayFromOriginAndDirection(origin, direction)

		epsilonOffset := 0.0
		if rand.Float64() < 0.25 {
			epsilonOffset = lastEpsilonOffset
		} else if rand.Float64() < 0.25 {
			epsilonOffset = 1e-3
		}
		ray.Advance(epsilonOffset)

		// test intersection
		intersection, hitFound := kdTree.Intersect(&ray)
		if hitFound {
			lastHit = ray.GetPoint(intersection.RayT)
			lastEpsilonOffset = intersection.RayTEpsilon
		}
	}

	benchmarkTime := float64(time.Since(start)) / float64(time.Second)
	speed := (float64(benchmarkRays) / 1000000.0) / benchmarkTime
	fmt.Printf("KdTree performance = %.2f MRays/sec\n", speed)
}
