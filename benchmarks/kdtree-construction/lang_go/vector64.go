package main

type Vector64 [3]float64

func VAdd64(v1, v2 Vector64) Vector64 {
	return Vector64{v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]}
}

func VSub64(v1, v2 Vector64) Vector64 {
	return Vector64{v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]}
}

func VMul64(v Vector64, s float64) Vector64 {
	return Vector64{v[0] * s, v[1] * s, v[2] * s}
}

func DotProduct64(v1 Vector64, v2 Vector64) float64 {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]
}
