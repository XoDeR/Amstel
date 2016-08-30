// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace Plane3Fn
{
	Plane3 createPlane3FromPointAndNormal(const Vector3& point, const Vector3& normal);
	// Normalizes the plane and returns the result
	Plane3& normalize(Plane3& p);
	// Returns the signed distance between the plane and the point
	float getDistanceToPoint(const Plane3& p, const Vector3& point);

} // namespace Plane3Fn

namespace Plane3Fn
{
	inline Plane3 createPlane3FromPointAndNormal(const Vector3& point, const Vector3& normal)
	{
		Plane3 p;
		p.n = normal;
		p.d = -dot(normal, point);
		return p;
	}

	inline Plane3& normalize(Plane3& p)
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

	inline float getDistanceToPoint(const Plane3& p, const Vector3& point)
	{
		return dot(p.n, point) + p.d;
	}
} // namespace Plane3Fn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka