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
    KdTree[] kdtrees;
    StopWatch sw;
    sw.start();
    foreach (mesh; meshes)
    {
        auto builder = KdTreeBuilder(mesh, KdTreeBuilder.BuildParams());
        kdtrees ~= builder.buildTree();
    }
    sw.stop();
    int elapsedTime = to!int(sw.peek().msecs());

    // validation
    assertEqualsHex(kdtrees[0].getHash(), 0xe044c3a15bbf0fe4,
        "model 0: invalid kdtree hash");
    assertEqualsHex(kdtrees[1].getHash(), 0xc3491ba1f8689922,
        "model 1: invalid kdtree hash");
    assertEqualsHex(kdtrees[2].getHash(), 0x255732f17a964439,
        "model 2: invalid kdtree hash");
    return elapsedTime;
}
