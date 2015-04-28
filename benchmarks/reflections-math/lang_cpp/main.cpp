#include "vector.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "../../../scripts/Timer.h"

std::vector<Vector> ReadNormals(const std::string& filename) {
    std::vector<Vector> normals;
    std::ifstream file(filename);
    if (file) {
        std::string line;
        std::string x_str, y_str, z_str;
        while (std::getline(file, line)) {
            std::stringstream stream(line);
            stream >> x_str >> y_str >> z_str;
            normals.push_back(Vector(std::stod(x_str), std::stod(y_str), std::stod(z_str)));
        }
    }
    return normals;
}

inline Vector ReflectVetor(const Vector& vector, const Vector& normal) {
    return 2.0 * DotProduct(vector, normal) * normal - vector;
}

int main(int argc, char* argv[]) {
    // prepare input data
    const std::string fileName = std::string(argv[1]) + "/normals.txt";
    auto normals = ReadNormals(fileName);

    // run benchmark
    Timer timer;
    auto vector = Vector(1, 0, 0);
    int count = 1024 * 1024 * 10;
    for (int i = 0; i < count; i++) {
        vector = ReflectVetor(vector, normals[i % normals.size()]);
    }
    const int elapsed_time = timer.elapsed() * 1000;

    // return benchmark results
    int exit_code = elapsed_time;
    return exit_code;
}
