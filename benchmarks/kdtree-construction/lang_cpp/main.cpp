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
  std::vector<KdTree> kdtrees;
  Timer timer;
  for (const auto& mesh : meshes) {
    auto builder = KdTreeBuilder(*mesh, KdTreeBuilder::BuildParams());
    kdtrees.push_back(builder.BuildTree());
  }
  const auto elapsedTime = timer.ElapsedMilliseconds();

  // validation
  AssertEquals(kdtrees[0].GetHash(), size_t(0x53b0b6a3d2f5281d),
               "model 0: invalid kdtree hash");
  AssertEquals(kdtrees[1].GetHash(), size_t(0x05a0ea0eb3581eee),
               "model 1: invalid kdtree hash");
  AssertEquals(kdtrees[2].GetHash(), size_t(0x23da8f7cbcbcf090),
               "model 2: invalid kdtree hash");
  return elapsedTime;
}
