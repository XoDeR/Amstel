// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Math/Aabb.h"

namespace Rio
{

namespace AabbFn
{
	void addPoints(Aabb& b, uint32_t pointsCount, uint32_t stride, const void* points)
	{
		const char* pts = (const char*)points;
		for (uint32_t i = 0; i < pointsCount; ++i, pts += stride)
		{
			const Vector3& pi = *(const Vector3*)pts;

			b.min.x = getMinFloat(b.min.x, pi.x);
			b.min.y = getMinFloat(b.min.y, pi.y);
			b.min.z = getMinFloat(b.min.z, pi.z);
			b.max.x = getMaxFloat(b.max.x, pi.x);
			b.max.y = getMaxFloat(b.max.y, pi.y);
			b.max.z = getMaxFloat(b.max.z, pi.z);
		}
	}

	void addBoxes(Aabb& b, uint32_t boxesCount, const Aabb* boxes)
	{
		for (uint32_t i = 0; i < boxesCount; ++i)
		{
			const Aabb& bi = boxes[i];

			b.min.x = getMinFloat(b.min.x, bi.min.x);
			b.min.y = getMinFloat(b.min.y, bi.min.y);
			b.min.z = getMinFloat(b.min.z, bi.min.z);
			b.max.x = getMaxFloat(b.max.x, bi.max.x);
			b.max.y = getMaxFloat(b.max.y, bi.max.y);
			b.max.z = getMaxFloat(b.max.z, bi.max.z);
		}
	}
} // namespace AabbFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka