package main

import "math"

func UniformSampleSphere(u1, u2 float64) Vector64 {
	z := 1.0 - 2.0 * u1
	r := math.Sqrt(1.0 - z*z)
	phi := 2.0 * math.Pi * u2
	x := r * math.Cos(phi)
	y := r * math.Sin(phi)
	return Vector64{x, y, z}
}
