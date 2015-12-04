
#include "kdtree.h"
#include "stl_loader.h"
#include "triangle_mesh.h"
#include "vector.h"
#include "common.h"

#include <memory>
#include <string>
#include <vector>

void benchmarkKdTree(const KdTree& kdtree);

std::string joinPath(std::string path1, std::string path2)
{
    if (!path1.empty() && (path1.back() == '/' || path1.back() == '\\'))
        path1 = path1.substr(0, path1.length() - 1);

    if (!path2.empty() && (path2[0] == '/' || path2[0] == '\\'))
        path2 = path2.substr(1, path2.length() - 1);

    return path1 + '/' + path2;
}

int main(int argc, char* argv[])
{
    enum { modelsCount = 3 };

    std::string modelFiles[modelsCount] =
    {
        joinPath(argv[1], "teapot.stl"),
        joinPath(argv[1], "bunny.stl"),
        joinPath(argv[1], "dragon.stl")
    };
    std::string kdtreeFiles[modelsCount] =
    {
        joinPath(argv[1], "teapot.kdtree"),
        joinPath(argv[1], "bunny.kdtree"),
        joinPath(argv[1], "dragon.kdtree")
    };

    std::vector<std::unique_ptr<TriangleMesh>> meshes;
    std::vector<std::unique_ptr<KdTree>> kdtrees;

    // load resources
    for (int i = 0; i < modelsCount; ++i)
    {
        meshes.push_back(loadStlFile(modelFiles[i]));
        kdtrees.push_back(std::make_unique<KdTree>(kdtreeFiles[i], *meshes.back()));
    }

    // run benchmark
    Timer timer;
    for (int i = 0; i < modelsCount; ++i)
    {
        benchmarkKdTree(*kdtrees[i]);
    }

    // return benchmark time in ms
    return timer.ElapsedSeconds() * 1000;
}
