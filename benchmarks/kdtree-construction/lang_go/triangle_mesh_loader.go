package main

import (
	"bytes"
	"common"
	"encoding/binary"
	"fmt"
	"math"
	"os"
)

func LoadTriangleMesh(fileName string) *TriangleMesh {
	const (
		headerSize        = 80
		facetSize         = 50
		maxVerticesCount  = math.MaxInt32
		maxTrianglesCount = math.MaxInt32
	)

	file, err := os.Open(fileName)
	common.Check(err)
	defer file.Close()

	// get file size
	stat, err := file.Stat()
	common.Check(err)
	fileSize := stat.Size()

	// read file content
	fileContent := make([]byte, fileSize)
	bytesRead, err := file.Read(fileContent)
	common.Check(err)

	if int64(bytesRead) != fileSize {
		common.RuntimeError(fmt.Sprintf("failed to read %d bytes from file %s",
			fileSize, fileName))
	}

	// validate file content
	asciiStlHeader := []byte{0x73, 0x6f, 0x6c, 0x69, 0x64}
	if bytes.Equal(fileContent[0:5], asciiStlHeader) {
		common.RuntimeError("ascii stl files are not supported: " + fileName)
	}

	if fileSize < headerSize+4 {
		common.RuntimeError("invalid binary stl file: " + fileName)
	}

	buffer := bytes.NewBuffer(fileContent[headerSize:])

	var trianglesCount int32
	err = binary.Read(buffer, binary.LittleEndian, &trianglesCount)
	common.Check(err)

	if trianglesCount > maxTrianglesCount {
		common.RuntimeError("triangles limit exceeded: " + fileName)
	}

	expectedSize := int64(headerSize + 4 + trianglesCount*facetSize)
	if fileSize != expectedSize {
		common.RuntimeError("invalid size of binary stl file: " + fileName)
	}

	// read mesh data
	mesh := new(TriangleMesh)
	mesh.normals = make([]Vector32, trianglesCount)
	mesh.triangles = make([][3]int32, trianglesCount)

	uniqueVertices := make(map[Vector32]int32)

	for i := 0; i < int(trianglesCount); i++ {
		binary.Read(buffer, binary.LittleEndian, &mesh.normals[i])

		for k := 0; k < 3; k++ {
			var v Vector32
			binary.Read(buffer, binary.LittleEndian, &v)
			vertexIndex, found := uniqueVertices[v]
			if !found {
				if len(mesh.vertices) > maxVerticesCount {
					common.RuntimeError("vertices limit exceeded: " + fileName)
				}
				vertexIndex = int32(len(mesh.vertices))
				uniqueVertices[v] = vertexIndex
				mesh.vertices = append(mesh.vertices, v)
			}
			mesh.triangles[i][k] = vertexIndex
		}
		var attribsCount uint16
		binary.Read(buffer, binary.LittleEndian, &attribsCount)
	}
	return mesh
}
