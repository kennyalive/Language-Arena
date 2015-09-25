package main

import (
	"fmt"
	"path"
	"os"
	"time"
)

func main() {
	modelFiles := []string{
		path.Join(os.Args[1], "teapot.stl"),
		path.Join(os.Args[1], "bunny.stl"),
		path.Join(os.Args[1], "dragon.stl"),
	}

	var meshes []*TriangleMesh
	for _, modelFile := range modelFiles {
		mesh, err := LoadStl(modelFile)
		if err != nil {
			fmt.Println("Error: %v", err)
			os.Exit(-1)
		}
		meshes = append(meshes, mesh)
	}

	// run benchmark
	start := time.Now()
	elapsedTime := int(time.Since(start) / time.Millisecond)
	os.Exit(elapsedTime)
}
