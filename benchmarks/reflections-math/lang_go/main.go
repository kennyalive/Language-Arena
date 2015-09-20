package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"time"
)

func ReadNormals(filePath string) (normals []Vector) {
	file, _ := os.Open(filePath)
	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)
	for scanner.Scan() {
		x, _ := strconv.ParseFloat(scanner.Text(), 64)
		scanner.Scan()
		y, _ := strconv.ParseFloat(scanner.Text(), 64)
		scanner.Scan()
		z, _ := strconv.ParseFloat(scanner.Text(), 64)
		normals = append(normals, Vector{x, y, z})
	}
	return
}

func ReflectVector(vector Vector, normal Vector) Vector {
	return VSub(vector, VMul(normal, 2.0*DotProduct(vector, normal)))
}

func RefractVector(vector Vector, normal Vector) Vector {
	const eta = 1.5
	nDotV := DotProduct(vector, normal)
	k := 1.0 - eta*eta*(1.0-nDotV*nDotV)
	if k < 0.0 {
		return Vector{0, 0, 0}
	} else {
		return VSub(VMul(vector, eta), VMul(normal, eta*nDotV+math.Sqrt(k)))
	}
}

func main() {
	// prepare input data
	fileName := os.Args[1] + "/normals.txt"
	normals := ReadNormals(fileName)

	// run benchmark
	start := time.Now()
	vector := Vector{1, 0, 0}
	const count = 1024 * 1024 * 100
	for i := 0; i < count; i++ {
		vector = ReflectVector(vector, normals[i%len(normals)])
		vector2 := RefractVector(vector, normals[(i+1)%len(normals)])
		if vector2 != (Vector{0, 0, 0}) {
			vector = vector2
		}
	}
	elapsedTime := int(time.Since(start) / time.Millisecond)

    if vector[0] + vector[1] + vector[2] == math.Pi {
        fmt.Println("matrix real")
    }

	//fmt.Printf("result: %.3f %.3f %.3f\n", vector[0], vector[1], vector[2])
	os.Exit(elapsedTime)
}
