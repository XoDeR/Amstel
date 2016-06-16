// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Math/Sphere.h"

namespace Rio
{

namespace SphereFn
{
	void addPoints(Sphere& s, uint32_t num, uint32_t stride, const void* points)
	{
		float rr = s.r*s.r;

		const char* pts = (const char*)points;
		for (uint32_t i = 0; i < num; ++i, pts += stride)
		{
			const Vector3& pi = *(const Vector3*)pts;
			rr = getMaxFloat(rr, getLengthSquared(pi - s.c));
		}

		s.r = sqrtf(rr);
	}

	void addSpheres(Sphere& s, uint32_t num, const Sphere* spheres)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const Sphere& si = spheres[i];
			const float dist = getLengthSquared(si.c - s.c);

			if (dist < (si.r + s.r) * (si.r + s.r))
			{
				if (si.r*si.r > s.r*s.r)
				{
					s.r = sqrtf(dist + si.r*si.r);
				}
			}
		}
	}
} // namespace SphereFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka