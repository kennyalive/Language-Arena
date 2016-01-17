import std.array;
import std.file;
import std.path;
import common;
import triangle_mesh;
import vector;

immutable(TriangleMesh) loadTriangleMesh(string fileName)
{
    if (!exists(fileName))
        runtimeError("file does not exist: " ~ fileName);

    if (extension(fileName) != ".stl")
        runtimeError("unknown file format: " ~ fileName);

    // first bytes of ascii stl ('solid')
    immutable(ubyte[5]) ascii_stl_header = [ 0x73, 0x6f, 0x6c, 0x69, 0x64 ];

    auto bytes = cast(ubyte[]) read(fileName, ascii_stl_header.length);
    if (bytes == ascii_stl_header)
        runtimeError("ascii stl files are not supported: " ~ fileName);

    return loadBinaryStlFile(fileName);
}

private immutable(TriangleMesh) loadBinaryStlFile(string fileName)
{
    enum headerSize = 84;
    enum facetSize = 50;

    void[] fileContent;
    try
    {
        fileContent = read(fileName);
    }
    catch (FileException)
    {
        runtimeError("failed to load stl file content: " ~ fileName);
    }

    if (fileContent.length < headerSize)
        runtimeError("invalid binary stl file: " ~ fileName);

    immutable uint numTriangles = *cast(uint*)(fileContent.ptr + headerSize - 4);

    if (fileContent.length < headerSize + numTriangles * facetSize)
        runtimeError("invalid binary stl file: " ~ fileName);

    auto mesh = new TriangleMesh();
    mesh.normals = new Vector_f[numTriangles];
    mesh.triangles = new TriangleMesh.Triangle[numTriangles];

    size_t[Vector_f] verticesSet;
    Appender!(Vector_f[]) verticesAppender;

    void* dataPtr = fileContent.ptr  + headerSize;
    foreach (i; 0..numTriangles)
    {
        float* f = cast(float*)dataPtr;
        mesh.normals[i] = Vector_f(f[0], f[1], f[2]);
        f += 3;

        foreach (k; 0..3)
        {
            auto v = Vector_f(f[0], f[1], f[2]);
            f += 3;

            size_t vIndex;
            auto p_index = v in verticesSet;
            if (p_index == null)
            {
                vIndex = verticesAppender.data.length;
                verticesSet[v] = vIndex;
                verticesAppender.put(v);
            }
            else
            {
                vIndex = *p_index;
            }
            mesh.triangles[i].points[k].vertexIndex = cast(int)vIndex;
        }
        dataPtr += facetSize;
    }
    mesh.vertices = verticesAppender.data;
    return cast(immutable(TriangleMesh)) mesh;
}
