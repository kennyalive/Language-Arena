import std.algorithm;
import std.math;
import std.random;
import std.stdio;

struct Vector
{
    double x, y, z;
}

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

void main()
{
    writeNormals(generateNormals());
}
