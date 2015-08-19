import core.memory;
import std.array;
import std.conv;
import std.datetime;
import std.exception;
import std.path;
import std.stdio;

import intersection;
import ray;
import vector;
import triangle_mesh;
import triangle_mesh_loader;
import kdtree;
import benchmark;

int main(string[] args)
{
    enum modelsCount = 3;

    string[modelsCount] modelFiles =
    [
        buildPath(args[1], "teapot.stl"),
        buildPath(args[1], "bunny.stl"),
        buildPath(args[1], "dragon.stl")
    ];

    string[modelsCount] kdtreeFiles =
    [
        buildPath(args[1], "teapot.kdtree"),
        buildPath(args[1], "bunny.kdtree"),
        buildPath(args[1], "dragon.kdtree")
    ];

    immutable(TriangleMesh)[] meshes;
    KdTree[] kdtrees;

    // load resources
    for (auto i = 0; i < modelsCount; ++i)
    {
        meshes ~= cast(immutable(TriangleMesh)) loadTriangleMesh(modelFiles[i]);
        kdtrees ~= KdTree(kdtreeFiles[i], meshes[i]);
    }

    // run benchmarks
    StopWatch sw;
    sw.start();
    foreach (ref kdtree; kdtrees)
    {
        benchmarkKdTree(kdtree);
    }
    sw.stop();
    int elapsedTime = to!int(sw.peek().msecs());

    return elapsedTime;
}
