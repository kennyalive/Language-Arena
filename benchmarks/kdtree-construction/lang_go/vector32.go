package main

type Vector32 [3]float32

func VAdd32(v1, v2 Vector32) Vector32 {
	return Vector32{v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]}
}

func VSub32(v1, v2 Vector32) Vector32 {
	return Vector32{v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]}
}

func VMul32(v Vector32, s float32) Vector32 {
	return Vector32{v[0] * s, v[1] * s, v[2] * s}
}

func DotProduct32(v1 Vector32, v2 Vector32) float32 {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]
}
