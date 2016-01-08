import std.math;
import vector;

struct Ray
{
    pure: nothrow: @nogc:

    this(Vector origin, Vector direction)
    {
        this.origin = origin;
        this.direction = direction;
    }

    @property Vector origin() const
    {
        return _origin;
    }

    @property void origin(Vector origin)
    {
        _origin = origin;
    }

    @property Vector direction() const
    {
        assert(_direction != Vector.init);
        return _direction;
    }

    @property Vector invDirection() const
    {
        assert(_invDirection != Vector.init);
        return _invDirection;
    }

    @property void direction(Vector direction)
    {
        assert(approxEqual(direction.length(), 1.0, 1e-6));
        _direction = direction;
        _invDirection = Vector(1.0 / direction.x,
                               1.0 / direction.y,
                               1.0 / direction.z);
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
