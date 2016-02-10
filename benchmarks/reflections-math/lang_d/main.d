import std.conv;
import std.datetime;
import std.file;
import std.math;
import std.path;
import std.string;
import common;
import vector;

private Vector[] readNormals(string filename)
{
    auto lines = readText(filename).splitLines();
    auto normals = new Vector[lines.length];
    foreach (i, line; lines)
    {
        auto coords = line.split();
        assert(coords.length == 3);
        normals[i] = Vector(to!double(coords[0]), to!double(coords[1]), to!double(coords[2]));
    }
    return normals;
}

private Vector reflectVector(Vector vector, Vector normal)
{
    return vector - 2.0 * dotProduct(vector, normal) * normal;
}

private Vector refractVector(Vector vector, Vector normal)
{
    immutable double eta = 0.7;
    double nDotV = dotProduct(vector, normal);
    double k = 1.0 - eta * eta * (1.0 - nDotV * nDotV);
    return eta * vector - (eta * nDotV + sqrt(k)) * normal;
}

int main(string[] args)
{
    // prepare input data
    string fileName = buildPath(args[1], "normals.txt");
    auto normals = readNormals(fileName);

    // run benchmark
    StopWatch sw;
    sw.start();
    auto vector = Vector(1, 0, 0);
    immutable int count = 1024 * 1024 * 100;

    foreach (i; 0..count)
    {
        vector = reflectVector(vector, normals[i % normals.length]);
        vector = refractVector(vector, normals[(i + 1) % normals.length]);
    }

    // communicate time to master
    int elapsedTime = to!int(sw.peek().msecs());
    const auto timingStorage = buildPath(dirName(args[0]), "timing");
    storeBenchmarkTiming(timingStorage, elapsedTime);

    // validation
    if (normals.length != 1024*1024) {
        validationError("invalid size of normals array");
    }
    if (!vector.equals(Vector(-0.2653, -0.1665, -0.9497), 1e-3)) {
        validationError("invalid final vector value");
    }
    return 0;
}
