package main

import "math"

type Vector64 [3]float64

func Vector64From32(v Vector32) Vector64 {
	return Vector64{float64(v[0]), float64(v[1]), float64(v[2])}
}

func Vector64FromScalar(s float64) Vector64 {
	return Vector64{s, s, s}
}

func VAdd64(v1, v2 Vector64) Vector64 {
	return Vector64{v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]}
}

func VSub64(v1, v2 Vector64) Vector64 {
	return Vector64{v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]}
}

func VMul64(v Vector64, s float64) Vector64 {
	return Vector64{v[0] * s, v[1] * s, v[2] * s}
}

func VLength64(v Vector64) float64 {
	return math.Sqrt(DotProduct64(v, v))
}

func VNormalized64(v Vector64) Vector64 {
	length := VLength64(v)
	return Vector64{v[0]/length, v[1]/length, v[2]/length}
}

func DotProduct64(v1, v2 Vector64) float64 {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]
}

func CrossProduct64(v1, v2 Vector64) Vector64 {
	return Vector64{
		v1[1]*v2[2] - v1[2]*v2[1],
		v1[2]*v2[0] - v1[0]*v2[2],
		v1[0]*v2[1] - v1[1]*v2[0],
	}
}
