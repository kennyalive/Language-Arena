package main

type Triangle struct {
	points [3]Vector64
}

type TriangleIntersection struct {
	t       float64
	epsilon float64
	b1      float64
	b2      float64
}

func IntersectTriangle(ray *Ray, triangle *Triangle) (bool, TriangleIntersection) {
	edge1 := VSub64(triangle.points[1], triangle.points[0])
	edge2 := VSub64(triangle.points[2], triangle.points[0])

	p := CrossProduct64(ray.GetDirection(), edge2)
	divisor := DotProduct64(edge1, p)

	// todo: do we need to check against epsilon for better numeric stability?
	if divisor == 0.0 {
		return false, TriangleIntersection{}
	}

	invDivisor := 1.0 / divisor

	// compute barycentric coordinate b1
	t := VSub64(ray.GetOrigin(), triangle.points[0])
	b1 := invDivisor * DotProduct64(t, p)
	if b1 < 0.0 || b1 > 1.0 {
		return false, TriangleIntersection{}
	}

	// compute barycentric coordinate b2
	q := CrossProduct64(t, edge1)
	b2 := invDivisor * DotProduct64(ray.GetDirection(), q)
	if b2 < 0.0 || b1+b2 > 1.0 {
		return false, TriangleIntersection{}
	}

	// compute distance from ray origin to intersection point
	distance := invDivisor * DotProduct64(edge2, q)
	if distance < 0.0 {
		return false, TriangleIntersection{}
	}

	return true, TriangleIntersection{
		t:       distance,
		epsilon: 1e-3 * distance,
		b1:      b1,
		b2:      b2,
	}
}
