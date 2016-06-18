// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace PlaneFn
{
	Plane createPlaneFromPointAndNormal(const Vector3& point, const Vector3& normal);
	// Normalizes the plane and returns the result
	Plane& normalize(Plane& p);
	// Returns the signed distance between the plane and the point
	float getDistanceToPoint(const Plane& p, const Vector3& point);

} // namespace PlaneFn

namespace PlaneFn
{
	inline Plane createPlaneFromPointAndNormal(const Vector3& point, const Vector3& normal)
	{
		Plane p;
		p.n = normal;
		p.d = -dot(normal, point);
		return p;
	}

	inline Plane& normalize(Plane& p)
	{
		const float length = getLength(p.n);

		if (getAreFloatsEqual(length, 0.0f))
		{
			return p;
		}

		const float invertedLength = 1.0f / length;

		p.n *= invertedLength;
		p.d *= invertedLength;

		return p;
	}

	inline float getDistanceToPoint(const Plane& p, const Vector3& point)
	{
		return dot(p.n, point) + p.d;
	}
} // namespace PlaneFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka