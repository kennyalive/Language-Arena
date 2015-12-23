#include "common.h"
#include "kdtree_builder.h"
#include "stl_loader.h"
#include "triangle_mesh.h"
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
    meshes.push_back(loadStlFile(modelFile));
  }

  // run benchmark
  std::vector<KdTree> kdtrees;
  Timer timer;
  for (const auto& mesh : meshes) {
    auto builder = KdTreeBuilder(*mesh, KdTreeBuilder::BuildParams());
    kdtrees.push_back(builder.buildTree());
  }
  const auto elapsedTime = static_cast<int>(timer.ElapsedSeconds() * 1000);

  // validation
  AssertEquals(kdtrees[0].getHash(), uint64_t(0x53b0b6a3d2f5281d),
               "model 0: invalid kdtree hash");
  AssertEquals(kdtrees[1].getHash(), uint64_t(0x05a0ea0eb3581eee),
               "model 1: invalid kdtree hash");
  AssertEquals(kdtrees[2].getHash(), uint64_t(0x23da8f7cbcbcf090),
               "model 2: invalid kdtree hash");
  return elapsedTime;
}
