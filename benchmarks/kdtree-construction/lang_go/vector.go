package main

type Vector [3]float64

func VAdd(v1, v2 Vector) Vector {
	return Vector{v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]}
}

func VSub(v1, v2 Vector) Vector {
	return Vector{v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]}
}

func VMul(v Vector, s float64) Vector {
	return Vector{v[0] * s, v[1] * s, v[2] * s}
}

func DotProduct(v1 Vector, v2 Vector) float64 {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]
}