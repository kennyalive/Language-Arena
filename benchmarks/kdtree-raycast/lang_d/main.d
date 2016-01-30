import std.path;
import std.stdio;
import benchmark;
import common;
import kdtree;
import random;
import triangle_mesh;
import triangle_mesh_loader;

int main(string[] args)
{
    enum modelsCount = 3;

    // prepare input data
    string[modelsCount] modelFiles = [
        buildPath(args[1], "teapot.stl"),
        buildPath(args[1], "bunny.stl"),
        buildPath(args[1], "dragon.stl")
    ];
    string[modelsCount] kdtreeFiles = [
        buildPath(args[1], "teapot.kdtree"),
        buildPath(args[1], "bunny.kdtree"),
        buildPath(args[1], "dragon.kdtree")
    ];

    immutable(TriangleMesh)[] meshes;
    immutable(KdTree)[] kdTrees;

    for (auto i = 0; i < modelsCount; ++i)
    {
        meshes ~= loadTriangleMesh(modelFiles[i]);
        kdTrees ~= new immutable(KdTree)(kdtreeFiles[i], meshes[i]);
    }

    // run benchmarks
    int elapsedTime = 0;
    foreach (i, kdTree; kdTrees)
    {
        int timeMsec = benchmarkKdTree(kdTree);
        elapsedTime += timeMsec;

        double speed = (benchmarkRaysCount / 1_000_000.0) / (timeMsec / 1000.0);
        writefln("raycast performance [%-6s]: %.2f MRays/sec",
            baseName(modelFiles[i], ".stl"), speed);
    }
    storeBenchmarkTiming(elapsedTime);

    // validation
    assertEquals(randUint(), 3404003823, "error in random generator");

    int[modelsCount] raysCount = [32768, 64, 32];
    foreach(i, kdTree; kdTrees)
        validateKdTree(kdTree, raysCount[i]);

    return 0;
}
