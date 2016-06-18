// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/Aabb.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/MathTypes.h"
#include "Core/Math/Plane.h"

namespace Rio
{

namespace FrustumFn
{
	// Builds the frustum <f> from the view matrix <m>
	void createFrustumFromMatrix(Frustum& f, const Matrix4x4& m);

	// Returns whether the frustum <f> contains the point <p>
	bool containsPoint(const Frustum& f, const Vector3& p);

	// Returns the corner <index> of the frustum <f>
	// Index to corner table:
	// 0 = Near bottom left
	// 1 = Near bottom right
	// 2 = Near top right
	// 3 = Near top left
	// 4 = Far bottom left
	// 5 = Far bottom right
	// 6 = Far top right
	// 7 = Far top left
	Vector3 getVertex(const Frustum& f, uint32_t index);
	// Returns the Aabb enclosing the frustum
	Aabb getAabb(const Frustum& f);
} // namespace FrustumFn

namespace FrustumFn
{
	inline void createFrustumFromMatrix(Frustum& f, const Matrix4x4& m)
	{
		f.left.n.x   = m.x.w + m.x.x;
		f.left.n.y   = m.y.w + m.y.x;
		f.left.n.z   = m.z.w + m.z.x;
		f.left.d     = m.t.w + m.t.x;

		f.right.n.x  = m.x.w - m.x.x;
		f.right.n.y  = m.y.w - m.y.x;
		f.right.n.z  = m.z.w - m.z.x;
		f.right.d    = m.t.w - m.t.x;

		f.bottom.n.x = m.x.w + m.x.y;
		f.bottom.n.y = m.y.w + m.y.y;
		f.bottom.n.z = m.z.w + m.z.y;
		f.bottom.d   = m.t.w + m.t.y;

		f.top.n.x    = m.x.w - m.x.y;
		f.top.n.y    = m.y.w - m.y.y;
		f.top.n.z    = m.z.w - m.z.y;
		f.top.d      = m.t.w - m.t.y;

		f.near.n.x   = m.x.w + m.x.z;
		f.near.n.y   = m.y.w + m.y.z;
		f.near.n.z   = m.z.w + m.z.z;
		f.near.d     = m.t.w + m.t.z;

		f.far.n.x    = m.x.w - m.x.z;
		f.far.n.y    = m.y.w - m.y.z;
		f.far.n.z    = m.z.w - m.z.z;
		f.far.d      = m.t.w - m.t.z;

		PlaneFn::normalize(f.left);
		PlaneFn::normalize(f.right);
		PlaneFn::normalize(f.bottom);
		PlaneFn::normalize(f.top);
		PlaneFn::normalize(f.near);
		PlaneFn::normalize(f.far);
	}

	inline bool containsPoint(const Frustum& f, const Vector3& p)
	{
		return !(PlaneFn::getDistanceToPoint(f.left, p) < 0.0f
			|| PlaneFn::getDistanceToPoint(f.right, p) < 0.0f
			|| PlaneFn::getDistanceToPoint(f.bottom, p) < 0.0f
			|| PlaneFn::getDistanceToPoint(f.top, p) < 0.0f
			|| PlaneFn::getDistanceToPoint(f.near, p) < 0.0f
			|| PlaneFn::getDistanceToPoint(f.far, p) < 0.0f
			);
	}

	inline Vector3 getVertex(const Frustum& f, uint32_t index)
	{
		RIO_ASSERT(index < 8, "Index out of bounds");

		// 0 = Near bottom left
		// 1 = Near bottom right
		// 2 = Near top right
		// 3 = Near top left
		// 4 = Far bottom left
		// 5 = Far bottom right
		// 6 = Far top right
		// 7 = Far top left

		const Plane* side = &f.left;
		Vector3 ip;

		switch (index)
		{
			case 0: getThreePlanesIntersection(side[4], side[0], side[2], ip); break;
			case 1: getThreePlanesIntersection(side[4], side[1], side[2], ip); break;
			case 2: getThreePlanesIntersection(side[4], side[1], side[3], ip); break;
			case 3: getThreePlanesIntersection(side[4], side[0], side[3], ip); break;
			case 4: getThreePlanesIntersection(side[5], side[0], side[2], ip); break;
			case 5: getThreePlanesIntersection(side[5], side[1], side[2], ip); break;
			case 6: getThreePlanesIntersection(side[5], side[1], side[3], ip); break;
			case 7: getThreePlanesIntersection(side[5], side[0], side[3], ip); break;
			default: break;
		}

		return ip;
	}

	inline Aabb getAabb(const Frustum& f)
	{
		Vector3 vertices[8];
		vertices[0] = getVertex(f, 0);
		vertices[1] = getVertex(f, 1);
		vertices[2] = getVertex(f, 2);
		vertices[3] = getVertex(f, 3);
		vertices[4] = getVertex(f, 4);
		vertices[5] = getVertex(f, 5);
		vertices[6] = getVertex(f, 6);
		vertices[7] = getVertex(f, 7);

		Aabb r;
		AabbFn::reset(r);
		AabbFn::addPoints(r, 8, vertices);
		return r;
	}
} // namespace FrustumFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka