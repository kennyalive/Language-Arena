#include "stl_loader.h"
#include "triangle_mesh.h"
#include "vector.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

TriangleMeshLoadingException::TriangleMeshLoadingException(const std::string& message)
: _message(message)
{
}

const char* TriangleMeshLoadingException::what() const
{
    return _message.c_str();
}

std::unique_ptr<TriangleMesh> loadStlFile(const std::string& fileName)
{
    enum
    {
        headerSize = 84,
        facetSize = 50
    };

    // Open stl file and parse header.
    std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);
    if (!file)
        throw TriangleMeshLoadingException("Failed to open file: " + fileName);

    file.seekg(0, std::ios_base::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios_base::beg);

    if (fileSize < headerSize)
        throw TriangleMeshLoadingException("Invalid stl file: " + fileName);

    std::vector<uint8_t> fileContent(fileSize);
    file.read(reinterpret_cast<char*>(fileContent.data()), fileSize);

    if (!file)
        throw TriangleMeshLoadingException("Failed to read file " + fileName);

    std::array<uint8_t, 5> asciiStlHeader = { 0x73, 0x6f, 0x6c, 0x69, 0x64 };
    if (std::memcmp(fileContent.data(), asciiStlHeader.data(), 5) == 0)
        throw TriangleMeshLoadingException("Ascii stl files are not supported: " + fileName);

    int32_t numTriangles = *reinterpret_cast<int32_t*>(fileContent.data() + headerSize - 4);
    if (fileContent.size() < headerSize + numTriangles * facetSize)
        throw TriangleMeshLoadingException("Invalid binary stl file: " + fileName);

    // Read mesh data.
    auto mesh = std::make_unique<TriangleMesh>();
    mesh->normals.resize(numTriangles);
    mesh->triangles.resize(numTriangles);

    struct VectorHash
    {
        std::size_t operator()(const Vector_f& v) const
        {
            size_t h1 = std::hash<float>()(v.x);
            size_t h2 = std::hash<float>()(v.y);
            size_t h3 = std::hash<float>()(v.z);
            return (h1 ^ (h2 << 1)) ^ h3;
        }
    };

    std::unordered_map<Vector_f, int32_t, VectorHash> uniqueVertices;
    uint8_t* dataPtr = fileContent.data() + headerSize;
    for (int32_t i = 0; i < numTriangles; ++i)
    {
        float* f = reinterpret_cast<float*>(dataPtr);
        mesh->normals[i] = Vector_f(f[0], f[1], f[2]);
        f += 3;

        for (int k = 0; k < 3; ++k)
        {
            Vector_f v(f[0], f[1], f[2]);
            f += 3;

            int32_t vertexIndex;

            auto iterator = uniqueVertices.find(v);
            if (iterator == uniqueVertices.cend())
            {
                if (mesh->vertices.size() > std::numeric_limits<int32_t>::max())
                    throw TriangleMeshLoadingException("Too large model: too many vertices");

                vertexIndex = static_cast<int32_t>(mesh->vertices.size());
                uniqueVertices[v] = vertexIndex;
                mesh->vertices.push_back(v);
            }
            else
            {
                vertexIndex = iterator->second;
            }
            mesh->triangles[i].vertexIndices[k] = vertexIndex;
        }
        dataPtr += facetSize;
    }
    std::vector<Vector_f>(mesh->vertices).swap(mesh->vertices);
    return mesh;
}
