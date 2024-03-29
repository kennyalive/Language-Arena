package main

import (
	"common"
	"os"
	"path"
	"path/filepath"
	"time"
)

func main() {
	// prepare input data
	modelFiles := []string{
		path.Join(os.Args[1], "teapot.stl"),
		path.Join(os.Args[1], "bunny.stl"),
		path.Join(os.Args[1], "dragon.stl"),
	}

	var meshes []*TriangleMesh
	for _, modelFile := range modelFiles {
		meshes = append(meshes, LoadTriangleMesh(modelFile))
	}

	// run benchmark
	start := time.Now()
	var kdTrees []*KdTree
	for _, mesh := range meshes {
		builder := NewKdTreeBuilder(mesh, NewBuildParams())
		kdTrees = append(kdTrees, builder.BuildKdTree())
	}

	// communicate time to master
	elapsedTime := int(time.Since(start) / time.Millisecond)
	timingStorage := path.Join(filepath.Dir(os.Args[0]), "timing")
	common.StoreBenchmarkTiming(timingStorage, elapsedTime)

	// validation
	common.AssertEqualsHex(kdTrees[0].GetHash(), 0xe044c3a15bbf0fe4,
		"model 0: invalid kdtree hash")
	common.AssertEqualsHex(kdTrees[1].GetHash(), 0xc3491ba1f8689922,
		"model 1: invalid kdtree hash")
	common.AssertEqualsHex(kdTrees[2].GetHash(), 0x255732f17a964439,
		"model 2: invalid kdtree hash")
}
