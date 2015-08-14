#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

#include "kdtree_builder.h"
#include "stl_loader.h"
#include "triangle_mesh.h"
#include "../../../scripts/Timer.h"

std::string joinPath(const std::string& path1, const std::string& path2)
{
    std::string path11 = path1;
    if (path11.back() == '/' || path11.back() == '\\')
        path11 = path11.substr(0, path11.length() - 1);

    std::string path22 = path2;
    if (path22[0] == '/' || path22[0] == '\\')
        path22 = path22.substr(1, path22.length() - 1);

    return path11 + '/' + path22;
}

int main(int argc, char* argv[]) {
    std::string modelFiles[] =
    {
        joinPath(argv[1], "bun_zipper_res4.stl"),
        joinPath(argv[1], "bun_zipper.stl"),
        joinPath(argv[1], "dragon_vrip.stl")
    };

    std::vector<std::unique_ptr<TriangleMesh>> meshes;
    for (const auto& modelFile : modelFiles)
    {
        meshes.push_back(loadStlFile(modelFile));
    }

    KdTreeBuilder::BuildParams buildParams;
    buildParams.collectStats = true;
    buildParams.leafCandidateTrianglesCount = 2;
    buildParams.emptyBonus = 0.3f;
    buildParams.splitAlongTheLongestAxis = false;

    std::vector<KdTree> kdtrees;
    std::vector<KdTreeBuilder::BuildStats> allStats;

    // run benchmark
    Timer timer;
    for (const auto& mesh : meshes)
    {
        auto builder = KdTreeBuilder(*mesh, buildParams);
        kdtrees.push_back(builder.buildTree());
        allStats.push_back(builder.getBuildStats());
    }
    const int elapsed_time = timer.elapsed() * 1000;

    // return benchmark results
    return elapsed_time;
}
