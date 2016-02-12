#include "benchmark.h"
#include "common.h"
#include "kdtree.h"
#include "random.h"
#include "triangle_mesh.h"
#include "triangle_mesh_loader.h"
#include "vector.h"
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  enum { modelsCount = 3 };

  // prepare input data
  std::string modelFiles[modelsCount] = {JoinPath(argv[1], "teapot.stl"),
                                         JoinPath(argv[1], "bunny.stl"),
                                         JoinPath(argv[1], "dragon.stl")};

  std::string kdtreeFiles[modelsCount] = {JoinPath(argv[1], "teapot.kdtree"),
                                          JoinPath(argv[1], "bunny.kdtree"),
                                          JoinPath(argv[1], "dragon.kdtree")};

  std::vector<std::unique_ptr<TriangleMesh>> meshes;
  std::vector<std::unique_ptr<KdTree>> kdTrees;

  for (int i = 0; i < modelsCount; i++) {
    meshes.push_back(LoadTriangleMesh(modelFiles[i]));
    kdTrees.push_back(
        std::unique_ptr<KdTree>(new KdTree(kdtreeFiles[i], *meshes.back())));
  }

  // run benchmark
  int elapsedTime = 0;
  for (int i = 0; i < modelsCount; i++) {
    int timeMsec = BenchmarkKdTree(*kdTrees[i]);
    elapsedTime += timeMsec;

    double speed = (benchmarkRaysCount / 1000000.0) / (timeMsec / 1000.0);
    printf("raycast performance [%-6s]: %.2f MRays/sec\n",
           StripExtension(GetFileName(modelFiles[i])).c_str(), speed);
  }

  // communicate time to master
  const auto timingStorage = JoinPath(GetDirectoryPath(argv[0]), "timing");
  StoreBenchmarkTiming(timingStorage, elapsedTime);

  // validation
  AssertEquals(RandUint32(), 3404003823u, "error in random generator");

  int raysCount[modelsCount] = {32768, 64, 32};
  for (int i = 0; i < modelsCount; i++) {
    ValidateKdTree(*kdTrees[i], raysCount[i]);
  }
  return 0;
}
