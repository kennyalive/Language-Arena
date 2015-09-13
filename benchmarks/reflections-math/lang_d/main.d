import vector;
import std.random;
import std.math;
import std.algorithm;
import std.datetime;
import std.conv;
import std.path;
import std.stdio;
import std.file;
import std.string;

private Vector[] generateNormals()
{
    immutable int count = 1024 * 1024;
    auto normals = new Vector[count];

    foreach (int i; 0..count)
    {
        double u = uniform(0.0, 1.0);
        double v = uniform(0.0, 1.0);

        double phi = 2.0 * PI * u;
        double theta = acos(max(-1.0, min(1.0, 2.0 * v - 1.0)));

        double sin_theta = sin(theta);
        normals[i] = Vector(cos(phi)*sin_theta, sin(phi)*sin_theta, cos(theta));
        assert(approxEqual(normals[i].length(), 1.0));
    }
    return normals;
}

private void writeNormals(Vector[] normals)
{
    auto f = File("normals.txt", "w");
    foreach (normal; normals)
    {
        f.writeln(normal.x, " ", normal.y, " ", normal.z);
    }
}

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
    immutable double eta = 1.5;
    double nDotV = dotProduct(vector, normal);
    double k = 1.0 - eta * eta * (1.0 - nDotV * nDotV);
    return (k < 0.0) ? Vector.init : eta * vector - (eta * nDotV + sqrt(k)) * normal;
}

int main(string[] args)
{
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
        auto vector2 = refractVector(vector, normals[(i + 1) % normals.length]);
        if (vector2 != Vector.init)
            vector = vector2;
    }
    sw.stop();
    int elapsedTime = to!int(sw.peek().msecs());

    // just to prevent compiler from optimizing vector calculations out
    if (vector.x + vector.y + vector.z == PI)
        writeln("matrix real");

    writefln("result: %.3f %.3f %.3f", vector.x, vector.y, vector.z);
    return elapsedTime;
}
