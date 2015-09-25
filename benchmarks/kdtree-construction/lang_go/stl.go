package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"os"
)

func LoadStl(fileName string) (*TriangleMesh, error) {
	const (
		headerSize = 84
		facetSize = 50
	)

	file, err := os.Open(fileName)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	stat, err := file.Stat()
	if err != nil {
		return nil, err
	}

	fileSize := stat.Size()
	if fileSize < headerSize {
		return nil, fmt.Errorf("Invalid stl file: %s", fileName)
	}

	fileContent := make([]byte, fileSize)
	bytesRead, err := file.Read(fileContent)
	if err != nil {
		return nil, err
	}
	if int64(bytesRead) != fileSize {
		return nil, fmt.Errorf("Failed to read %d bytes from file %s", fileSize, fileName)
	}

	asciiStlHeader := []byte{0x73, 0x6f, 0x6c, 0x69, 0x64}
	if bytes.Equal(fileContent[0:5], asciiStlHeader) {
		return nil, fmt.Errorf("Ascii stl files are not supported: %s", fileName)
	}

	buffer := bytes.NewBuffer(fileContent[headerSize - 4:])

	var trianglesCount int32
	err = binary.Read(buffer, binary.LittleEndian, &trianglesCount)
	if err != nil {
		return nil, err
	}
	if fileSize < int64(headerSize + trianglesCount * facetSize) {
		return nil, fmt.Errorf("Invalid binary stl file: %s", fileName)
	}

	mesh := new(TriangleMesh)
	mesh.normals = make([]Vector, trianglesCount)
	mesh.triangles = make([][3]int32, trianglesCount)

	uniqueVertices := make(map[Vector]int32)

	for i := int32(0); i < trianglesCount; i++ {
		binary.Read(buffer, binary.LittleEndian, &mesh.normals[i])

		for k := 0; k < 3; k++ {
			var v Vector
			binary.Read(buffer, binary.LittleEndian, &v)
			vertexIndex, found := uniqueVertices[v]
			if !found {
				if len(mesh.vertices) > math.MaxInt32 {
					return nil, fmt.Errorf("Too large model: %s", fileName)
				}
				vertexIndex = int32(len(mesh.vertices))
				uniqueVertices[v] = vertexIndex
				mesh.vertices = append(mesh.vertices, v)
			}
			mesh.triangles[i][k] = vertexIndex
		}
	}
	return mesh, nil
}
