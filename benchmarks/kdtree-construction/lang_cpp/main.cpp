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
  AssertEqualsHex(kdtrees[0].GetHash(), uint64_t(0xe044c3a15bbf0fe4),
               "model 0: invalid kdtree hash");
  AssertEqualsHex(kdtrees[1].GetHash(), uint64_t(0xc3491ba1f8689922),
               "model 1: invalid kdtree hash");
  AssertEqualsHex(kdtrees[2].GetHash(), uint64_t(0x255732f17a964439),
               "model 2: invalid kdtree hash");
  return elapsedTime;
}
