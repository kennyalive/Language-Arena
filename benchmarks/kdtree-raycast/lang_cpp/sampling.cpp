#include "vector.h"

#include <cassert>
#include <cmath>

const float PI = 3.14159265358979323846f;

Vector uniformSampleSphere(double u1, double u2)
{
    assert(u1 >= 0.0 && u1 < 1.0);
    assert(u2 >= 0.0 && u2 < 1.0);

    double z = 1.0 - 2.0 * u1;
    double r = sqrt(1.0 - z*z);
    double phi = 2.0 * PI * u2;
    double x = r * cos(phi);
    double y = r * sin(phi);
    return Vector(x, y, z);
}
