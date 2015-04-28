#pragma once

#include <cmath>

template <typename T>
struct TVector {
    T x, y, z;

    TVector()
        : x(0), y(0), z(0)
    {}

    TVector(T _x, T _y, T _z)
        : x(_x), y(_y), z(_z)
    {}

    TVector operator+(const TVector& v) const {
        return TVector(x + v.x, y + v.y, z + v.z);
    }

    TVector operator-(const TVector& v) const {
        return TVector(x - v.x, y - v.y, z - v.z);
    }

    TVector operator*(T value) const {
        return TVector(x * value, y * value, z * value);
    }

    TVector operator/(T value) const {
        const T inv_value = 1.0 / value;
        return TVector(x * inv_value, y * inv_value, z * inv_value);
    }

    T Length2() const {
        return x*x + y*y + z*z;
    }

    T Length() const {
        return sqrt(Length2());
    }

    TVector GetNormalized() const {
        return *this / Length();
    }
};

using Vector = TVector<double>;
using Vector_f = TVector<float>;

template <typename T>
inline TVector<T> operator*(T value, const TVector<T>& v) {
    return v * value;
}

template <typename T>
inline T DotProduct(const TVector<T>& a, const TVector<T>& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

template <typename T>
inline TVector<T> CrossProduct(const TVector<T>& a, const TVector<T>& b) {
    return TVector<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
