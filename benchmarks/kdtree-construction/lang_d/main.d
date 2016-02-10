import std.conv;
import std.datetime;
import std.path;
import common;
import kdtree;
import kdtree_builder;
import triangle_mesh;
import triangle_mesh_loader;
import vector;

int main(string[] args)
{
    // prepare input data
    string[] modelFiles = [
        buildPath(args[1], "teapot.stl"),
        buildPath(args[1], "bunny.stl"),
        buildPath(args[1], "dragon.stl")
    ];

    immutable(TriangleMesh)[] meshes;
    foreach (modelFile; modelFiles)
        meshes ~= cast(immutable(TriangleMesh)) loadTriangleMesh(modelFile);

    // run benchmark
    StopWatch sw;
    sw.start();
    KdTree[] kdTrees;
    foreach (mesh; meshes)
    {
        auto builder = KdTreeBuilder(mesh, KdTreeBuilder.BuildParams());
        kdTrees ~= builder.buildTree();
    }

    // communicate time to master
    int elapsedTime = to!int(sw.peek().msecs());
    const auto timingStorage = buildPath(dirName(args[0]), "timing");
    storeBenchmarkTiming(timingStorage, elapsedTime);

    // validation
    assertEqualsHex(kdTrees[0].getHash(), 0xe044c3a15bbf0fe4,
        "model 0: invalid kdtree hash");
    assertEqualsHex(kdTrees[1].getHash(), 0xc3491ba1f8689922,
        "model 1: invalid kdtree hash");
    assertEqualsHex(kdTrees[2].getHash(), 0x255732f17a964439,
        "model 2: invalid kdtree hash");
    return 0;
}
