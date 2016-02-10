package main

import (
	"common"
	"fmt"
	"os"
	"path"
	"path/filepath"
)

func main() {
	const modelsCount = 3

	// prepare input data
	modelFiles := [modelsCount]string{
		path.Join(os.Args[1], "teapot.stl"),
		path.Join(os.Args[1], "bunny.stl"),
		path.Join(os.Args[1], "dragon.stl"),
	}

	kdTreeFiles := [modelsCount]string{
		path.Join(os.Args[1], "teapot.kdtree"),
		path.Join(os.Args[1], "bunny.kdtree"),
		path.Join(os.Args[1], "dragon.kdtree"),
	}

	var meshes []*TriangleMesh
	var kdTrees []*KdTree

	for i := 0; i < modelsCount; i++ {
		mesh := LoadTriangleMesh(modelFiles[i])
		meshes = append(meshes, mesh)

		kdTree := NewKdTree(kdTreeFiles[i], mesh)
		kdTrees = append(kdTrees, kdTree)
	}

	// run benchmark
	elapsedTime := 0
	for i, kdTree := range kdTrees {
		timeMsec := BenchmarkKdTree(kdTree)
		elapsedTime += timeMsec

		speed := (float64(BenchmarkRaysCount) / 1000000.0) / (float64(timeMsec) / 1000.0)
		baseName := path.Base(modelFiles[i])
		fmt.Printf("raycast performance [%-6s] = %.2f MRays/sec\n",
			baseName[:len(baseName)-4], speed)
	}

	// communicate time to master
	timingStorage := path.Join(filepath.Dir(os.Args[0]), "timing")
	common.StoreBenchmarkTiming(timingStorage, elapsedTime)

	// validation
	common.AssertEquals(uint64(RandUint32()), 3404003823, "error in random generator")

	raysCount := [modelsCount]int{32768, 64, 32}
	for i := 0; i < modelsCount; i++ {
		ValidateKdTree(kdTrees[i], raysCount[i])
	}
}
