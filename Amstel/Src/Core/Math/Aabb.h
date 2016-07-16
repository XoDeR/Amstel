// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Math/MathTypes.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace AabbFn
{
	void reset(Aabb& b);
	Vector3 getCenter(const Aabb& b);
	float getRadius(const Aabb& b);
	float getVolume(const Aabb& b);
	// Adds <pointsCount> <points> to the box <b>, expanding its bounds if necessary
	void addPoints(Aabb& a, uint32_t pointsCount, uint32_t stride, const void* points);
	// Adds <pointsCount> <points> to the box <b>, expanding its bounds if necessary
	void addPoints(Aabb& b, uint32_t pointsCount, const Vector3* points);
	// Adds <boxesCount> <boxes> to the box <b>, expanding its bounds if necessary
	void addBoxes(Aabb& b, uint32_t boxesCount, const Aabb* boxes);
	// Returns whether point <p> is contained in the box <b>
	bool containsPoint(const Aabb& b, const Vector3& p);
	// Returns the <index> -th vertex of the box
	Vector3 getVertex(const Aabb& b, uint32_t index);
	// Returns the box enclosing <b> transformed by <m>
	Aabb getTransformed(const Aabb& b, const Matrix4x4& m);
	// Returns the eight vertices of the box <b>
	void getVertices(const Aabb& b, Vector3 v[8]);
	// Returns the sphere enclosing the box <b>
	Sphere getSphere(const Aabb& b);
}

namespace AabbFn
{
	inline void reset(Aabb& b)
	{
		b.min = VECTOR3_ZERO;
		b.max = VECTOR3_ZERO;
	}

	inline Vector3 getCenter(const Aabb& b)
	{
		return (b.min + b.max) * 0.5f;
	}

	inline float getRadius(const Aabb& b)
	{
		return getLength(b.max - (b.min + b.max) * 0.5f);
	}

	inline float getVolume(const Aabb& b)
	{
		return (b.max.x - b.min.x) * (b.max.y - b.min.y) * (b.max.z - b.min.z);
	}

	inline void addPoints(Aabb& b, uint32_t pointsCount, const Vector3* points)
	{
		AabbFn::addPoints(b, pointsCount, sizeof(Vector3), points);
	}

	inline bool containsPoint(const Aabb& b, const Vector3& p)
	{
		return p.x > b.min.x
			&& p.y > b.min.y
			&& p.z > b.min.z
			&& p.x < b.max.x
			&& p.y < b.max.y
			&& p.z < b.max.z
			;
	}

	inline Vector3 getVertex(const Aabb& b, uint32_t index)
	{
		RIO_ASSERT(index < 8, "Index out of bounds");

		switch (index)
		{
			case 0: return createVector3(b.min.x, b.min.y, b.min.z);
			case 1: return createVector3(b.max.x, b.min.y, b.min.z);
			case 2: return createVector3(b.max.x, b.min.y, b.max.z);
			case 3: return createVector3(b.min.x, b.min.y, b.max.z);
			case 4: return createVector3(b.min.x, b.max.y, b.min.z);
			case 5: return createVector3(b.max.x, b.max.y, b.min.z);
			case 6: return createVector3(b.max.x, b.max.y, b.max.z);
			case 7: return createVector3(b.min.x, b.max.y, b.max.z);
			default: return VECTOR3_ZERO;
		}
	}

	inline Aabb getTransformed(const Aabb& b, const Matrix4x4& m)
	{
		Vector3 vertices[8];
		getVertices(b, vertices);

		vertices[0] = vertices[0] * m;
		vertices[1] = vertices[1] * m;
		vertices[2] = vertices[2] * m;
		vertices[3] = vertices[3] * m;
		vertices[4] = vertices[4] * m;
		vertices[5] = vertices[5] * m;
		vertices[6] = vertices[6] * m;
		vertices[7] = vertices[7] * m;

		Aabb r;
		AabbFn::reset(r);
		AabbFn::addPoints(r, 8, vertices);
		return r;
	}

	inline void getVertices(const Aabb& b, Vector3 v[8])
	{
		// 7 ---- 6
		// |      |
		// |      |  <--- Top face
		// 4 ---- 5

		// 3 ---- 2
		// |      |
		// |      |  <--- Bottom face
		// 0 ---- 1
		v[0].x = b.min.x;
		v[0].y = b.min.y;
		v[0].z = b.max.z;

		v[1].x = b.max.x;
		v[1].y = b.min.y;
		v[1].z = b.max.z;

		v[2].x = b.max.x;
		v[2].y = b.min.y;
		v[2].z = b.min.z;

		v[3].x = b.min.x;
		v[3].y = b.min.y;
		v[3].z = b.min.z;

		v[4].x = b.min.x;
		v[4].y = b.max.y;
		v[4].z = b.max.z;

		v[5].x = b.max.x;
		v[5].y = b.max.y;
		v[5].z = b.max.z;

		v[6].x = b.max.x;
		v[6].y = b.max.y;
		v[6].z = b.min.z;

		v[7].x = b.min.x;
		v[7].y = b.max.y;
		v[7].z = b.min.z;
	}

	inline Sphere getSphere(const Aabb& b)
	{
		Sphere s;
		s.c = AabbFn::getCenter(b);
		s.r = AabbFn::getRadius(b);
		return s;
	}
} // namespace AabbFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka