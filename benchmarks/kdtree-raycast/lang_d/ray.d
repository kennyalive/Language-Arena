import std.math;
import vector;

struct Ray
{
    pure: nothrow: @nogc:

    this(Vector origin, Vector direction)
    {
        assert(approxEqual(direction.length(), 1.0, 1e-6));
        _origin = origin;
        _direction = direction;
        _invDirection = Vector(1.0 / direction.x,
                               1.0 / direction.y,
                               1.0 / direction.z);
    }

    @property Vector origin() const
    {
        return _origin;
    }

    @property Vector direction() const
    {
        return _direction;
    }

    @property Vector invDirection() const
    {
        return _invDirection;
    }

    void advance(double t)
    {
        _origin = getPoint(t);
    }

    Vector getPoint(double t) const
    {
        return _origin + _direction * t;
    }

private:
    Vector _origin;
    Vector _direction;
    Vector _invDirection;
}
