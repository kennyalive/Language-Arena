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
        _invDirection = Vector(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
    }

    void advance(double t)
    {
        _origin = getPoint(t);
    }

    Vector getPoint(double t)
    {
        return _origin + _direction * t;
    }

private:
    Vector _origin;
    Vector _direction;
    Vector _invDirection;
}

unittest
{
    auto origin = Vector(1, 2, 3);
    auto direction = Vector(1, 1, 1).getNormalized();

    auto ray = Ray(origin, direction);

    assert(ray.origin == origin);
    assert(ray.direction == direction);

    auto invDirection = Vector(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
    assert(ray.invDirection == invDirection);
}

unittest
{
    auto origin = Vector(0.1, 0, -1);
    auto direction = Vector(1, 2, 3).getNormalized();

    Ray ray;
    ray.origin = origin;
    ray.direction = direction;

    assert(ray.origin == origin);
    assert(ray.direction == direction);

    auto invDirection = Vector(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
    assert(ray.invDirection == invDirection);
}

unittest
{
    auto ray = Ray(Vector.init, Vector(0, 1, 0));

    assert(ray.origin == Vector.init);
    assert(ray.direction == Vector(0, 1, 0));

    assert(ray.invDirection.x == double.infinity);
    assert(ray.invDirection.y == 1.0);
    assert(ray.invDirection.z == double.infinity);
}
