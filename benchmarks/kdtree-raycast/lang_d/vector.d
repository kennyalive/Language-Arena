import std.math;

alias Vector = TVector!double;
alias Vector_f = TVector!float;

struct TVector(T)
{
    pure: nothrow: @nogc:

    T x = 0;
    T y = 0;
    T z = 0;

    this(T x, T y, T z = 0)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    this(T value)
    {
        this(value, value, value);
    }

    this(T2)(TVector!T2 other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
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

    ref TVector!T opOpAssign(string op)(TVector v)
    if (op == "+")
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return this;
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
    return TVector!T(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}
