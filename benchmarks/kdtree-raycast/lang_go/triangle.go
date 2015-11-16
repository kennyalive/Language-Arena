package main

type Triangle [3]Vector64

type TriangleIntersection struct {
	t          float64
	b1         float64
	b2         float64
	rayEpsilon float64
}

func IntersectTriangle(ray *Ray, triangle *Triangle) (intersection TriangleIntersection, hitFound bool) {
	edge1 := VSub64(triangle[1], triangle[0])
	edge2 := VSub64(triangle[2], triangle[0])

	p := CrossProduct64(ray.GetDirection(), edge2)
	divisor := DotProduct64(edge1, p)

	if divisor == 0.0 {
		return
	}

	invDivisor := 1.0 / divisor

	// compute barycentric coordinate b1
	t := VSub64(ray.GetOrigin(), triangle[0])
	b1 := invDivisor * DotProduct64(t, p)
	if b1 < 0.0 || b1 > 1.0 {
		return
	}

	// compute barycentric coordinate b2
	q := CrossProduct64(t, edge1)
	b2 := invDivisor * DotProduct64(ray.GetDirection(), q)
	if b2 < 0.0 || b1+b2 > 1.0 {
		return
	}

	// compute distance from ray origin to intersection point
	distance := invDivisor * DotProduct64(edge2, q)
	if distance < 0.0 {
		return
	}

	rayEpsilon := 1e-3 * distance

	return TriangleIntersection{
		distance,
		b1,
		b2,
		rayEpsilon,
	}, true
}
