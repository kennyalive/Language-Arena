import std.math;

alias Vector   = TVector!double;
alias Vector_f = TVector!float;

struct TVector(T)
{
    T x = 0;
    T y = 0;
    T z = 0;

    this (T _x, T _y, T _z = 0)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    this(T value)
    {
        this(value, value, value);
    }

    ref T opIndex(size_t i)
    {
        assert(i >= 0 && i < 3);
        return (&x)[i];
    }

    T opIndex(size_t i) const
    {
        assert(i >= 0 && i < 3);
        return (&x)[i];
    }

    TVector opUnary(string op)() const
    if (op == "-")
    {
        return TVector(-x, -y, -z);
    }

    TVector opBinary(string op)(TVector v) const
    if (op == "+") 
    {
        return TVector(x + v.x, y + v.y, z + v.z);
    }

    TVector opBinary(string op)(TVector v) const
    if (op == "-") 
    {
        return TVector(x - v.x, y - v.y, z - v.z);
    }

    TVector opBinary(string op)(T value) const
    if (op == "*") 
    {
        return TVector(x * value, y * value, z * value);
    }

    TVector opBinaryRight(string op)(T value) const
    if (op == "*") 
    {
        return this.opBinary!op(value);
    }

    TVector opBinary(string op)(T value) const
    if (op == "/") 
    {
        immutable T inv_value = 1.0 / value;
        return TVector(x * inv_value, y * inv_value, z * inv_value);
    }

    T length2() const 
    {
        return x*x + y*y + z*z;
    }

    T length() const 
    {
        return sqrt(length2());
    }

    TVector getNormalized() const 
    {
        return this / length();
    }
}

T dotProduct(T)(TVector!T a, TVector!T b) 
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

TVector!T crossProduct(T)(TVector!T a, TVector!T b) 
{
    return TVector!T(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

version(unittest) 
{
    immutable auto eps = 1e-6;
}

// construction
unittest {
    Vector v;
    assert(v.x == 0);
    assert(v.y == 0);
    assert(v.z == 0);

    Vector v2 = Vector(1, 2, 3);
    assert(v2.x == 1);
    assert(v2.y == 2);
    assert(v2.z == 3);

    Vector v3 = Vector(-1);
    assert(v3.x == -1);
    assert(v3.y == -1);
    assert(v3.z == -1);

    auto v4 = Vector(3.14, 2);
    assert(abs(v4.x - 3.14) < eps);
    assert(v4.y == 2);
    assert(v4.z == 0);
}

// vector negation
unittest {
    auto v = Vector(1, 0, -2.1);
    Vector v_neg = -v;
    assert(v_neg.x == -1);
    assert(v_neg.y == 0);
    assert(abs(v_neg.z - 2.1) < eps);
}

// vector + vector
unittest {
    Vector v = Vector(1, 2, 3) + Vector(3.14, -1.0);
    assert(abs(v.x - 4.14) < eps);
    assert(v.y == 1);
    assert(v.z == 3);
}

// vector - vector
unittest {
    Vector a = Vector(3.14, -1);
    Vector v = Vector(1, 2, 3) - a;
    assert(abs(v.x + 2.14) < eps);
    assert(v.y == 3);
    assert(v.z == 3);
}

// vector * scalar
unittest {
    auto a = Vector(1, 2, 3);
    Vector b = a * 2.5;
    assert(abs(b.x - 2.5) < eps);
    assert(abs(b.y - 5.0) < eps);
    assert(abs(b.z - 7.5) < eps);
}

// scalar * vector
unittest {
    auto a = Vector(1, 2, 3);
    Vector b = -2.5 * a;
    assert(abs(b.x - -2.5) < eps);
    assert(abs(b.y - -5.0) < eps);
    assert(abs(b.z - -7.5) < eps);
}

// vector / scalar
unittest {
    auto a = Vector(1.1, 0, -1024);
    Vector b = a / 2;
    assert(abs(b.x - 0.55) < eps);
    assert(abs(b.y -    0) < eps);
    assert(abs(b.z - -512) < eps);
}

// length2
unittest {
    assert(approxEqual(Vector(0, 0, 0).length2(), 0.0, eps));
    assert(approxEqual(Vector(1, 2, 3).length2(), 14, eps));
    assert(approxEqual(Vector(1.5, 2, 0.1).length2(), 6.26, eps));
}

// length
unittest {
    assert(approxEqual(Vector(1, 1, 1).length(), sqrt(3.0), eps));
    assert(approxEqual(Vector(3, 4).length(), 5, eps));
    assert(approxEqual(Vector(1, 0, 0).length(), 1, eps));
}

// getNormalized
unittest {
    assert(approxEqual(Vector(1, 2, 3).getNormalized().length(), 1, eps));
    
    auto v = Vector(3, 0, 4);
    Vector n = v.getNormalized();
    assert(approxEqual(n.x, 0.6, eps));
    assert(approxEqual(n.y, 0.0, eps));
    assert(approxEqual(n.z, 0.8, eps));
}

// dot product
unittest {
    double dot = dotProduct(Vector(0.5, 1, 10), Vector(1, 2, 3));
    assert(abs(dot - 32.5) < eps);
}

// cross product
unittest {
    auto v = crossProduct(Vector(1, 0, 0), Vector(0, 1, 0));
    assert(v.x == 0);
    assert(v.y == 0);
    assert(v.z == 1);

    auto v2 = crossProduct(Vector(1, 2, 3), Vector(2, 4, 6));
    assert(v2.x == 0);
    assert(v2.y == 0);
    assert(v2.z == 0);

    auto v3 = crossProduct(Vector(1, 0, -4), Vector(1, 2, 3));
    assert(v3.x == 8);
    assert(v3.y == -7);
    assert(v3.z == 2);
}
