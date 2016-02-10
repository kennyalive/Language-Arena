package main

import (
	"bufio"
	"common"
	"math"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"time"
)

func ReadNormals(fileName string) []Vector {
	file, err := os.Open(fileName)
	common.Check(err)
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)

	var normals []Vector

	for scanner.Scan() {
		x, err := strconv.ParseFloat(scanner.Text(), 64)
		common.Check(err)

		scanner.Scan()
		y, err := strconv.ParseFloat(scanner.Text(), 64)
		common.Check(err)

		scanner.Scan()
		z, err := strconv.ParseFloat(scanner.Text(), 64)
		common.Check(err)

		normals = append(normals, Vector{x, y, z})
	}
	common.Check(scanner.Err())
	return normals
}

func ReflectVector(vector Vector, normal Vector) Vector {
	return VSub(vector, VMul(normal, 2.0*DotProduct(vector, normal)))
}

func RefractVector(vector Vector, normal Vector) Vector {
	const eta = 0.7
	nDotV := DotProduct(vector, normal)
	k := 1.0 - eta*eta*(1.0-nDotV*nDotV)
	return VSub(VMul(vector, eta), VMul(normal, eta*nDotV+math.Sqrt(k)))
}

func main() {
	// prepare input data
	fileName := path.Join(os.Args[1], "normals.txt")
	normals := ReadNormals(fileName)

	// run benchmark
	start := time.Now()
	vector := Vector{1, 0, 0}
	const count = 1024 * 1024 * 100

	for i := 0; i < count; i++ {
		vector = ReflectVector(vector, normals[i%len(normals)])
		vector = RefractVector(vector, normals[(i+1)%len(normals)])
	}

	// communicate time to master
	elapsedTime := int(time.Since(start) / time.Millisecond)
	timingStorage := path.Join(filepath.Dir(os.Args[0]), "timing")
	common.StoreBenchmarkTiming(timingStorage, elapsedTime)

	// validation
	if len(normals) != 1024*1024 {
		common.ValidationError("invalid size of normals array")
	}
	if !VIsEqual(vector, Vector{-0.2653, -0.1665, -0.9497}, 1e-3) {
		common.ValidationError("invalid final vector value")
	}
}
