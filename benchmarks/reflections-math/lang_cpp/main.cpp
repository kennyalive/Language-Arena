#include "vector.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "common.h"

std::vector<Vector> ReadNormals(const std::string& filename)
{
    std::vector<Vector> normals;
    std::ifstream file(filename);
    if (file)
    {
        std::string line;
        std::string x_str, y_str, z_str;
        while (std::getline(file, line))
        {
            std::stringstream stream(line);
            stream >> x_str >> y_str >> z_str;
            normals.push_back(Vector(std::stod(x_str), std::stod(y_str), std::stod(z_str)));
        }
    }
    return normals;
}

inline Vector ReflectVector(const Vector& vector, const Vector& normal)
{
    return vector - 2.0 * DotProduct(vector, normal) * normal;
}

inline Vector RefractVector(const Vector& vector, const Vector& normal)
{
    static const double eta = 1.5;
    double nDotV = DotProduct(vector, normal);
    double k = 1.0 - eta * eta * (1.0 - nDotV * nDotV);
    return (k < 0.0) ? Vector() : eta * vector - (eta * nDotV + std::sqrt(k)) * normal;
}

int main(int argc, char* argv[])
{
    // prepare input data
    const std::string fileName = std::string(argv[1]) + "/normals.txt";
    auto normals = ReadNormals(fileName);

    // run benchmark
    Timer timer;
    auto vector = Vector(1, 0, 0);
    int count = 1024 * 1024 * 100;
    for (int i = 0; i < count; i++)
    {
        vector = ReflectVector(vector, normals[i % normals.size()]);
        auto vector2 = RefractVector(vector, normals[(i + 1) % normals.size()]);
        if (vector2 != Vector())
            vector = vector2;
    }
    const int elapsed_time = timer.ElapsedSeconds() * 1000;

    // just to prevent compiler from optimizing vector calculations out
    if (vector.x + vector.y + vector.z == 3.14159)
        printf("matrix real");

    //printf("result: %.3f %.3f %.3f\n", vector.x, vector.y, vector.z);
    return elapsed_time;
}
