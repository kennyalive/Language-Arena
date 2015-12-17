#include "common.h"
#include "kdtree_builder.h"
#include "stl_loader.h"
#include "triangle_mesh.h"
#include <memory>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    std::string modelFiles[] = {JoinPath(argv[1], "teapot.stl"),
                                JoinPath(argv[1], "bunny.stl"),
                                JoinPath(argv[1], "dragon.stl")};

    std::vector<std::unique_ptr<TriangleMesh>> meshes;
    for (const auto& modelFile : modelFiles) {
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
    for (const auto& mesh : meshes) {
        auto builder = KdTreeBuilder(*mesh, buildParams);
        kdtrees.push_back(builder.buildTree());
        allStats.push_back(builder.getBuildStats());
    }
    const int elapsed_time = timer.ElapsedSeconds() * 1000;

    // return benchmark results
    return elapsed_time;
}
