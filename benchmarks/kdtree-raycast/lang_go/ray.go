package main

type Ray struct {
	origin Vector64
	direction Vector64
	invDirection Vector64
}

func RayFromOriginAndDirection(origin, direction Vector64) Ray {
	ray := Ray{
		origin: origin,
	}
	ray.SetDirection(direction)
	return ray
}

func (ray *Ray) GetOrigin() Vector64 {
	return ray.origin
}

func (ray *Ray) SetOrigin(origin Vector64) {
	ray.origin = origin
}

func (ray *Ray) GetDirection() Vector64 {
	return ray.direction
}

func (ray *Ray) GetInvDirection() Vector64 {
	return ray.invDirection
}

func (ray *Ray) SetDirection(direction Vector64) {
	ray.direction = direction
	ray.invDirection = Vector64{1.0/direction[0], 1.0/direction[1], 1.0/direction[2]}
}

func (ray *Ray) Advance(t float64) {
	ray.origin = ray.GetPoint(t)
}

func (ray *Ray) GetPoint(t float64) Vector64 {
	return VAdd64(ray.origin, VMul64(ray.direction, t))
}
