package main

import (
	"common"
	"fmt"
	"math"
	"time"
)

const BenchmarkRaysCount = 10000000

func uniformSampleSphere() Vector64 {
	u1 := RandFloat64()
	u2 := RandFloat64()
	z := 1.0 - 2.0*u1
	r := math.Sqrt(1.0 - z*z)
	phi := 2.0 * math.Pi * u2
	x := r * math.Cos(phi)
	y := r * math.Sin(phi)
	return Vector64{x, y, z}
}

type rayGenerator struct {
	raysBounds BBox64
}

func newRayGenerator(meshBounds BBox64) *rayGenerator {
	diagonal := VSub64(meshBounds.maxPoint, meshBounds.minPoint)
	delta := 2.0 * VLength64(diagonal)

	raysBounds := NewBBox64FromPoints(
		VSub64(meshBounds.minPoint, NewVector64FromScalar(delta)),
		VAdd64(meshBounds.maxPoint, NewVector64FromScalar(delta)),
	)

	return &rayGenerator{
		raysBounds: raysBounds,
	}
}

func (rg *rayGenerator) generateRay(lastHit Vector64, lastHitEpsilon float64) Ray {
	// generate ray origin
	origin := Vector64{
		RandForRange(rg.raysBounds.minPoint[0], rg.raysBounds.maxPoint[0]),
		RandForRange(rg.raysBounds.minPoint[1], rg.raysBounds.maxPoint[1]),
		RandForRange(rg.raysBounds.minPoint[2], rg.raysBounds.maxPoint[2]),
	}

	useLastHit := RandFloat64() < 0.25
	if useLastHit {
		origin = lastHit
	}

	// generate ray direction
	direction := uniformSampleSphere()

	if RandFloat64() < 1.0/32.0 && direction[2] != 0.0 {
		direction[0] = 0
		direction[1] = 0
	} else if RandFloat64() < 1.0/32.0 && direction[1] != 0.0 {
		direction[0] = 0
		direction[2] = 0
	} else if RandFloat64() < 1.0/32.0 && direction[0] != 0.0 {
		direction[1] = 0
		direction[2] = 0
	}
	direction = VNormalized64(direction)

	// initialize ray
	ray := RayFromOriginAndDirection(origin, direction)

	if useLastHit {
		ray.Advance(lastHitEpsilon)
	} else {
		ray.Advance(1e-3)
	}
	return ray
}

func BenchmarkKdTree(kdTree *KdTree) int {
	start := time.Now()

	lastHit := VMul64(VAdd64(kdTree.meshBounds.minPoint, kdTree.meshBounds.maxPoint), 0.5)
	lastHitEpsilon := 0.0

	rg := newRayGenerator(kdTree.meshBounds)

	for raysTested := 0; raysTested < BenchmarkRaysCount; raysTested++ {
		ray := rg.generateRay(lastHit, lastHitEpsilon)

		hitFound, intersection := kdTree.Intersect(&ray)
		if hitFound {
			lastHit = ray.GetPoint(intersection.t)
			lastHitEpsilon = intersection.epsilon
		}

		// // debug output
		// if raysTested < 1024 {
		// 	if hitFound {
		// 		fmt.Printf("%d: found: %v, lastHit: %.14f %.14f %.14f\n",
		// 			raysTested, hitFound, lastHit[0], lastHit[1], lastHit[2])
		// 	} else {
		// 		fmt.Printf("%d: found: %v\n", raysTested, hitFound)
		// 	}
		// }
	}
	return int(time.Since(start) / time.Millisecond)
}

func ValidateKdTree(kdTree *KdTree, raysCount int) {
	lastHit := VMul64(VAdd64(kdTree.meshBounds.minPoint, kdTree.meshBounds.maxPoint), 0.5)
	lastHitEpsilon := 0.0

	rg := newRayGenerator(kdTree.meshBounds)

	for raysTested := 0; raysTested < raysCount; raysTested++ {
		ray := rg.generateRay(lastHit, lastHitEpsilon)

		kdTreeHitFound, kdTreeIntersection := kdTree.Intersect(&ray)

		bruteForceIntersection := KdTreeIntersection{t: math.Inf(+1)}
		bruteForceHitFound := false

		for i := int32(0); i < kdTree.mesh.GetTrianglesCount(); i++ {
			indices := kdTree.mesh.triangles[i]

			triangle := Triangle{[3]Vector64{
				NewVector64FromVector32(kdTree.mesh.vertices[indices[0]]),
				NewVector64FromVector32(kdTree.mesh.vertices[indices[1]]),
				NewVector64FromVector32(kdTree.mesh.vertices[indices[2]]),
			}}

			hitFound, intersection := IntersectTriangle(&ray, &triangle)

			if hitFound && intersection.t < bruteForceIntersection.t {
				bruteForceIntersection.t = intersection.t
				bruteForceHitFound = true
			}
		}

		if kdTreeHitFound != bruteForceHitFound ||
			kdTreeIntersection.t != bruteForceIntersection.t {
			o := ray.origin
			d := ray.direction
			fmt.Printf("KdTree accelerator test failure:\n"+
				"KdTree hit: %v\n"+
				"actual hit: %v\n"+
				"KdTree T %.16g [%b]\n"+
				"actual T %.16g [%b]\n"+
				"ray origin: (%b, %b, %b)\n"+
				"ray direction: (%b, %b, %b)\n",
				kdTreeHitFound, bruteForceHitFound,
				kdTreeIntersection.t, kdTreeIntersection.t,
				bruteForceIntersection.t, bruteForceIntersection.t,
				o[0], o[1], o[2], d[0], d[1], d[2])
			common.ValidationError("kdTree traversal error detected")
		}

		if bruteForceHitFound {
			lastHit = ray.GetPoint(bruteForceIntersection.t)
			lastHitEpsilon = bruteForceIntersection.epsilon
		}
	}
}
