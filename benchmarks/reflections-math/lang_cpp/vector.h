#pragma once

#include <cassert>
#include <cmath>

template <typename T>
struct TVector {
    T x, y, z;

    TVector()
        : x(0), y(0), z(0)
    {}

    TVector(T value)
        : x(value)
        , y(value)
        , z(value)
    {
    }

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

    bool operator==(const TVector& v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }

    bool operator!=(const TVector& v) const
    {
        return !(*this == v);
    }

    T operator[](int index) const
    {
        assert(index >= 0 && index < 3);
        return (&x)[index];
    }

    T& operator[](int index)
    {
        assert(index >= 0 && index < 3);
        return (&x)[index];
    }

    T Length2() const {
        return x*x + y*y + z*z;
    }

    T Length() const {
        return std::sqrt(Length2());
    }

    TVector GetNormalized() const {
        return *this / Length();
    }

    bool Equals(const TVector& v, T epsilon) const
    {
        return std::abs(x - v.x) <= epsilon &&
               std::abs(y - v.y) <= epsilon &&
               std::abs(z - v.z) <= epsilon;
    }
};

using Vector   = TVector<double>;
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
