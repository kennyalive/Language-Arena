package main

import (
	"log"
	"os"
	"path"
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
			log.Fatal(err)
		}
		meshes = append(meshes, mesh)
	}

	var kdtrees []*KdTree

	buildParams := NewBuildParams()
	buildParams.CollectStats = true
	buildParams.LeafCandidateTrianglesCount = 2
	buildParams.EmptyBonus = 0.3
	buildParams.SplitAlongTheLongestAxis = false

	// run benchmark
	start := time.Now()
	for _, mesh := range meshes {
		builder, err := NewKdTreeBuilder(mesh, buildParams)
		if err != nil {
			log.Fatal(err)
		}
		kdtree := builder.BuildKdTree()
		kdtrees = append(kdtrees, kdtree)
		//builder.buildStats.Print()
	}
	elapsedTime := int(time.Since(start) / time.Millisecond)
	os.Exit(elapsedTime)
}
