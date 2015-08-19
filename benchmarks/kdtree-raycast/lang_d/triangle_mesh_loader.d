import std.algorithm;
import std.array;
import std.conv;
import std.file;
import std.path;

import triangle_mesh;
import vector;

class TriangleMeshLoadingException : Exception 
{
    pure nothrow
    this(string message)
    {
        super(message);
    }

    pure nothrow
    this(string message, Exception cause)
    {
        super(message, cause);
    }
}

TriangleMesh loadTriangleMesh(string fileName)
{
    if (!exists(fileName))
        throw new TriangleMeshLoadingException("File does not exist: " ~ fileName);

    if (extension(fileName) != ".stl")
        throw new TriangleMeshLoadingException("Unknown file format: " ~ fileName);

    immutable(ubyte[5]) ascii_stl_header = [ 0x73, 0x6f, 0x6c, 0x69, 0x64 ]; // 'solid'
    auto bytes = cast(ubyte[]) read(fileName, ascii_stl_header.length);

    try
    {
        if (bytes == ascii_stl_header) 
            return loadAsciiStlFile(fileName);
        else
            return loadBinaryStlFile(fileName);
    }
    catch (FileException e)
    {
        throw new TriangleMeshLoadingException("Failed to load triangle mesh due to file IO error");
    }
    catch (ConvException e)
    {
        throw new TriangleMeshLoadingException("Failed to load triangle mesh due to conversion error");
    }
}

private TriangleMesh loadAsciiStlFile(string fileName)
{
    auto fileContent = read(fileName);
    auto fileContentRange = splitter(cast(string)fileContent, '\n');

    // We reuse already parsed memory space to store parsed data.
    // Ascii STL format guarantees that parsed data will never overlap with text that is not parsed yet.
    float* dataBuffer = cast(float*)fileContent;
    float* dataBufferIterator = dataBuffer; // current location to store data

    enum max_words_per_line = 5; // at most 5 tokens per line
    string[max_words_per_line] wordsBuf; 
    int wordsCount;

    void readNextLine()
    {
        wordsCount = 0;
        while (wordsCount == 0)
        {
            if (fileContentRange.empty)
                throw new TriangleMeshLoadingException("Unexpected end of STL file");

            string line = fileContentRange.front();
            fileContentRange.popFront();

            int wordFound = false;
            size_t wordStart;
            foreach (i, c; line)
            {
                if (c <= 32) // whitespace
                {
                    if (wordFound)
                    {
                        wordsBuf[wordsCount++] = line[wordStart..i];
                        wordFound = false;

                        if (wordsCount == max_words_per_line)
                            break;
                    }
                }
                else
                {
                    if (!wordFound)
                    {
                        wordStart = i;
                        wordFound = true;
                    }
                }
            }
            if (wordFound)
                wordsBuf[wordsCount++] = line[wordStart..$];
        }
    }

    readNextLine();
    if (wordsBuf[0] != "solid")
        throw new TriangleMeshLoadingException("Ascii stl file should start with solid keyword: " ~ fileName);

    readNextLine();
    while (wordsBuf[0] != "endsolid")
    {
        if (wordsCount != 5 || wordsBuf[0] != "facet" || wordsBuf[1] != "normal") 
            throw new TriangleMeshLoadingException("Expect 'facet normal nx ny nz'");

        *dataBufferIterator++ = to!float(wordsBuf[2]);
        *dataBufferIterator++ = to!float(wordsBuf[3]);
        *dataBufferIterator++ = to!float(wordsBuf[4]);

        readNextLine();
        if (wordsCount != 2 || wordsBuf[0] != "outer" || wordsBuf[1] != "loop")
            throw new TriangleMeshLoadingException("Expect 'outer loop'");

        foreach (i; 0..3)
        {
            readNextLine();
            if (wordsCount != 4 || wordsBuf[0] != "vertex")
                throw new TriangleMeshLoadingException("Expect 'vertex vx vy vz'");

            *dataBufferIterator++ = to!float(wordsBuf[1]);
            *dataBufferIterator++ = to!float(wordsBuf[2]);
            *dataBufferIterator++ = to!float(wordsBuf[3]);
        }

        readNextLine();
        if (wordsBuf[0] != "endloop")
            throw new TriangleMeshLoadingException("Expect 'endloop'");
        readNextLine();
        if (wordsBuf[0] != "endfacet")
            throw new TriangleMeshLoadingException("Expect 'endfacet'");
        readNextLine();
    }

    immutable dataBufferSize = dataBufferIterator - dataBuffer;
    assert(dataBufferSize % 12 == 0);
    immutable numTriangles = dataBufferSize / 12;

    auto mesh = new TriangleMesh();
    mesh.normals = new Vector_f[numTriangles];
    mesh.triangles = new TriangleMesh.Triangle[numTriangles];

    size_t[Vector_f] verticesSet;
    Appender!(Vector_f[]) verticesAppender;

    dataBufferIterator = dataBuffer;
    foreach (i; 0..numTriangles)
    {
        mesh.normals[i] = Vector_f(dataBufferIterator[0], dataBufferIterator[1], dataBufferIterator[2]);
        dataBufferIterator += 3;

        foreach (k; 0..3)
        {
            auto v = Vector_f(dataBufferIterator[0], dataBufferIterator[1], dataBufferIterator[2]);
            dataBufferIterator += 3;

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
            mesh.triangles[i].vertex_indices[k] = cast(int)vIndex;
        }
    }
    mesh.vertices = verticesAppender.data;
    return mesh;
}

private TriangleMesh loadBinaryStlFile(string fileName)
{
    enum headerSize = 84;
    enum facetSize = 50;

    auto fileContent = read(fileName);
    if (fileContent.length < headerSize)
        throw new TriangleMeshLoadingException("Invalid binary stl file: " ~ fileName);

    immutable uint numTriangles = *cast(uint*)(fileContent.ptr + headerSize - 4);

    if (fileContent.length < headerSize + numTriangles * facetSize)
        throw new TriangleMeshLoadingException("Invalid binary stl file: " ~ fileName);

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
            mesh.triangles[i].vertex_indices[k] = cast(int)vIndex;
        }
        dataPtr += facetSize;
    }
    mesh.vertices = verticesAppender.data;
    return mesh;
}
