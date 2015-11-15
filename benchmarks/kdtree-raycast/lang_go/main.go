package main

import (
	"log"
	"os"
	"path"
	"time"
)

func checkError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

func main() {
	const modelsCount = 3

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

	// load resources
	var meshes []*TriangleMesh
	var kdTrees []*KdTree

	for i := 0; i < modelsCount; i++ {
		mesh, err := LoadStl(modelFiles[i])
		if err != nil {
			log.Fatal(err)
		}
		meshes = append(meshes, mesh)

		kdTree := NewKdTree(kdTreeFiles[i], mesh)
		kdTrees = append(kdTrees, kdTree)
 	}

	// run benchmark
	start := time.Now()
	for _, kdTree := range kdTrees {
		benchmarkKdTree(kdTree)
	}
	elapsedTime := int(time.Since(start) / time.Millisecond)
	os.Exit(elapsedTime)
}
