#include "common.h"
#include "triangle_mesh.h"
#include "triangle_mesh_loader.h"
#include "vector.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>

std::unique_ptr<TriangleMesh> LoadTriangleMesh(const std::string& fileName)
{
  enum {
    headerSize = 80,
    facetSize = 50,
    maxVerticesCount = static_cast<size_t>(std::numeric_limits<int32_t>::max()),
    maxTrianglesCount = static_cast<size_t>(std::numeric_limits<int32_t>::max())
  };

  struct VectorHash {
    std::size_t operator()(const Vector_f& v) const
    {
      size_t h1 = std::hash<float>()(v.x);
      size_t h2 = std::hash<float>()(v.y);
      size_t h3 = std::hash<float>()(v.z);
      return CombineHashes(h1, CombineHashes(h2, h3));
    }
  };

  std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);
  if (!file)
    RuntimeError("failed to open file: " + fileName);

  // get file size
  file.seekg(0, std::ios_base::end);
  auto fileSize = file.tellg();
  file.seekg(0, std::ios_base::beg);

  if (fileSize == std::streampos(-1) || !file)
    RuntimeError("failed to read file stats: " + fileName);

  // read file content
  std::vector<uint8_t> fileContent(static_cast<size_t>(fileSize));
  file.read(reinterpret_cast<char*>(fileContent.data()), fileSize);
  if (!file)
    RuntimeError("failed to read file content: " + fileName);

  // validate file content
  std::array<uint8_t, 5> asciiStlHeader = {0x73, 0x6f, 0x6c, 0x69, 0x64};
  if (memcmp(fileContent.data(), asciiStlHeader.data(), 5) == 0)
    RuntimeError("ascii stl files are not supported: " + fileName);

  if (fileSize < headerSize + 4)
    RuntimeError("invalid binary stl file: " + fileName);

  uint32_t numTriangles =
      *reinterpret_cast<uint32_t*>(fileContent.data() + headerSize);

  if (numTriangles > maxTrianglesCount)
    RuntimeError("too large model: too many triangles: " + fileName);

  auto expectedSize =
      headerSize + 4 + static_cast<size_t>(numTriangles) * facetSize;

  if (fileContent.size() != expectedSize)
    RuntimeError("incorrect size of binary stl file: " + fileName);

  // read mesh data
  auto mesh = std::unique_ptr<TriangleMesh>(new TriangleMesh());
  mesh->normals.resize(numTriangles);
  mesh->triangles.resize(numTriangles);

  std::unordered_map<Vector_f, int32_t, VectorHash> uniqueVertices;
  uint8_t* dataPtr = fileContent.data() + headerSize + 4;
  for (uint32_t i = 0; i < numTriangles; i++) {
    float* f = reinterpret_cast<float*>(dataPtr);
    mesh->normals[i] = Vector_f(f[0], f[1], f[2]);
    f += 3;

    for (int k = 0; k < 3; ++k) {
      Vector_f v(f[0], f[1], f[2]);
      f += 3;

      int32_t vertexIndex;

      auto iterator = uniqueVertices.find(v);
      if (iterator == uniqueVertices.cend()) {
        if (mesh->vertices.size() > maxVerticesCount) {
          RuntimeError("too large model: too many vertices");
        }
        vertexIndex = static_cast<int32_t>(mesh->vertices.size());
        uniqueVertices[v] = vertexIndex;
        mesh->vertices.push_back(v);
      }
      else {
        vertexIndex = iterator->second;
      }
      mesh->triangles[i].points[k].vertexIndex = vertexIndex;
    }
    dataPtr += facetSize;
  }
  std::vector<Vector_f>(mesh->vertices).swap(mesh->vertices);
  return mesh;
}
