#include "common.h"
#include "vector.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<Vector> ReadNormals(const std::string& fileName) {
  std::ifstream file(fileName);
  if (!file)
    RuntimeError("failed to open file " + fileName);

  std::vector<Vector> normals;
  std::string line;
  while (std::getline(file, line)) {
    std::stringstream stream(line);
    std::string x_str, y_str, z_str;
    stream >> x_str >> y_str >> z_str;
    normals.push_back(
        Vector(std::stod(x_str), std::stod(y_str), std::stod(z_str)));
  }
  return normals;
}

inline Vector ReflectVector(const Vector& vector, const Vector& normal) {
  return vector - 2.0 * DotProduct(vector, normal) * normal;
}

inline Vector RefractVector(const Vector& vector, const Vector& normal) {
  static const double eta = 0.7;
  double nDotV = DotProduct(vector, normal);
  double k = 1.0 - eta * eta * (1.0 - nDotV * nDotV);
  return eta * vector - (eta * nDotV + std::sqrt(k)) * normal;
}

int main(int argc, char* argv[]) {
  // prepare input data
  const auto fileName = JoinPath(argv[1], "normals.txt");
  const auto normals = ReadNormals(fileName);

  // run benchmark
  Timer timer;
  auto vector = Vector(1, 0, 0);
  const int count = 1024 * 1024 * 100;

  for (int i = 0; i < count; i++) {
    vector = ReflectVector(vector, normals[i % normals.size()]);
    vector = RefractVector(vector, normals[(i + 1) % normals.size()]);
  }
  const auto elapsedTime = timer.ElapsedMilliseconds();
  StoreBenchmarkTiming(elapsedTime);

  // validation
  if (normals.size() != 1024 * 1024) {
    ValidationError("invalid size of normals array");
  }
  if (!vector.Equals(Vector(-0.2653, -0.1665, -0.9497), 1e-3)) {
    ValidationError("invalid final vector value");
  }
  return 0;
}
