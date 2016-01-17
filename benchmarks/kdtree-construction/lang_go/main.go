package main

import (
	"log"
	"os"
	"path"
	"time"
	"common"
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
		mesh, err := LoadStl(modelFile)
		if err != nil {
			log.Fatal(err)
		}
		meshes = append(meshes, mesh)
	}

	// run benchmark
	var kdtrees []*KdTree
	start := time.Now()
	for _, mesh := range meshes {
		builder, err := NewKdTreeBuilder(mesh,  NewBuildParams())
		if err != nil {
			log.Fatal(err)
		}
		kdtree := builder.BuildKdTree()
		kdtrees = append(kdtrees, kdtree)
	}
	elapsedTime := int(time.Since(start) / time.Millisecond)

	// validation
	common.AssertEqualsHex(kdtrees[0].GetHash(), 0xe044c3a15bbf0fe4,
		"model 0: invalid kdtree hash")
  	common.AssertEqualsHex(kdtrees[1].GetHash(), 0xc3491ba1f8689922,
   		"model 1: invalid kdtree hash")
  	common.AssertEqualsHex(kdtrees[2].GetHash(), 0x255732f17a964439,
         "model 2: invalid kdtree hash")
	os.Exit(elapsedTime)
}
