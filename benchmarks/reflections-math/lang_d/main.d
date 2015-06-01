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

private Vector[] generateNormals() {
    immutable int count = 1024 * 1024;
    auto normals = new Vector[count];

    foreach (int i; 0..count) {
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

private void writeNormals(Vector[] normals) {
    auto f = File("normals.txt", "w");
    foreach (normal; normals) {
        f.writeln(normal.x, " ", normal.y, " ", normal.z);
    }
}

private Vector[] readNormals(string filename) {
    auto lines = readText(filename).splitLines();
    auto normals = new Vector[lines.length];
    foreach (i, line; lines) {
        auto coords = line.split();
        assert(coords.length == 3);
        normals[i] = Vector(to!double(coords[0]), to!double(coords[1]), to!double(coords[2]));
    }
    return normals;
}

private Vector reflectVector(Vector vector, Vector normal) {
    return 2.0 * dotProduct(vector, normal) * normal - vector;
}

int main(string[] args)
{
    string fileName = buildPath(args[1], "normals.txt");
    auto normals = readNormals(fileName);

    // run benchmark
    StopWatch sw;
    sw.start();
    auto vector = Vector(1, 0, 0);
    immutable int count = 1024 * 1024 * 10;
    foreach (i; 0..count) {
        vector = reflectVector(vector, normals[i % normals.length]);
    }
    sw.stop();
    writeln(vector.x, vector.y, vector.z);
    int elapsedTime = to!int(sw.peek().msecs());

    // return benchmark results
    int exitCode = elapsedTime;
    return exitCode;
}
