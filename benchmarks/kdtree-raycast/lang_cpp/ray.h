#pragma once

#include "vector.h"

#include <cmath>
#include <cstdlib>

class Ray
{
public:
    Ray(const Vector& origin, const Vector& direction)
        : _origin(origin)
    {
        setDirection(direction);
    }

    const Vector& getOrigin() const
    {
        return _origin;
    }

    void setOrigin(const Vector& origin)
    {
        _origin = origin;
    }

    const Vector& getDirection() const
    {
        assert(_direction != Vector());
        return _direction;
    }

    const Vector& getInvDirection() const
    {
        assert(_invDirection != Vector());
        return _invDirection;
    }

    void setDirection(const Vector& direction)
    {
        assert(std::abs(direction.Length() - 1.0) < 1e-6);
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
};
