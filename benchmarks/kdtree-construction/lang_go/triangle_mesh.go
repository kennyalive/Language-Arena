package main

type TriangleMesh struct {
	vertices  []Vector32
	normals   []Vector32
	triangles [][3]int32
}

func (mesh *TriangleMesh) GetTrianglesCount() int32 {
	return int32(len(mesh.triangles))
}

func (mesh *TriangleMesh) GetTriangleBounds(triangleIndex int32) BBox32 {
	indices := mesh.triangles[triangleIndex]
	bbox := NewBBox32FromPoint(mesh.vertices[indices[0]])
	bbox.Extend(mesh.vertices[indices[1]])
	bbox.Extend(mesh.vertices[indices[2]])
	return bbox
}

func (mesh *TriangleMesh) GetBounds() BBox32 {
	meshBounds := NewBBox32()
	for i := 0; i < len(mesh.triangles); i++ {
		meshBounds = BBox32Union(meshBounds, mesh.GetTriangleBounds(int32(i)))
	}
	return meshBounds
}
