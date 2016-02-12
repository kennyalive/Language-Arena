#include "common.h"
#include "kdtree_builder.h"
#include "triangle_mesh.h"
#include "triangle_mesh_loader.h"
#include <memory>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  // prepare input data
  std::string modelFiles[] = {JoinPath(argv[1], "teapot.stl"),
                              JoinPath(argv[1], "bunny.stl"),
                              JoinPath(argv[1], "dragon.stl")};

  std::vector<std::unique_ptr<TriangleMesh>> meshes;
  for (const auto& modelFile : modelFiles) {
    meshes.push_back(LoadTriangleMesh(modelFile));
  }

  // run benchmark
  Timer timer;
  std::vector<KdTree> kdTrees;
  for (const auto& mesh : meshes) {
    auto builder = KdTreeBuilder(*mesh, KdTreeBuilder::BuildParams());
    kdTrees.push_back(builder.BuildTree());
  }

  // communicate time to master
  const auto elapsedTime = timer.ElapsedMilliseconds();
  const auto timingStorage = JoinPath(GetDirectoryPath(argv[0]), "timing");
  StoreBenchmarkTiming(timingStorage, elapsedTime);

  // validation
  AssertEqualsHex(kdTrees[0].GetHash(), uint64_t(0xe044c3a15bbf0fe4),
                  "model 0: invalid kdtree hash");
  AssertEqualsHex(kdTrees[1].GetHash(), uint64_t(0xc3491ba1f8689922),
                  "model 1: invalid kdtree hash");
  AssertEqualsHex(kdTrees[2].GetHash(), uint64_t(0x255732f17a964439),
                  "model 2: invalid kdtree hash");
  return 0;
}
