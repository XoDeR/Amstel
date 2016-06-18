// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Math/Aabb.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

float getRayPlaneIntersection(const Vector3& from, const Vector3& dir, const Plane& p)
{
	const float num = dot(from, p.n);
	const float den = dot(dir, p.n);

	if (getAreFloatsEqual(den, 0.0f))
	{
		return -1.0f;
	}

	return (-p.d - num) / den;
}

float getRayDiscIntersection(const Vector3& from, const Vector3& dir, const Vector3& center, float radius, const Vector3& normal)
{
	const Plane p = PlaneFn::createPlaneFromPointAndNormal(center, normal);
	const float t = getRayPlaneIntersection(from, dir, p);

	if (t == -1.0f)
	{
		return -1.0f;
	}

	const Vector3 intersectionPoint = from + dir * t;
	if (getDistanceSquared(intersectionPoint, center) < radius*radius)
	{
		return t;
	}

	return -1.0f;
}

float getRaySphereIntersection(const Vector3& from, const Vector3& dir, const Sphere& s)
{
	const Vector3 v = s.c - from;
	const float b   = dot(v, dir);
	const float rr  = s.r * s.r;
	const float bb  = b * b;
	const float det = rr - dot(v, v) + bb;

	if (det < 0.0f || b < s.r)
	{
		return -1.0f;
	}

	return b - sqrtf(det);
}

float getRayObbIntersection(const Vector3& from, const Vector3& dir, const Matrix4x4& transformMatrix, const Vector3& halfExtents)
{
	float tMin = 0.0f;
	float tMax = 999999999.9f;

	const Vector3 obbPosition = createVector3(transformMatrix.t.x, transformMatrix.t.y, transformMatrix.t.z);
	const Vector3 delta = obbPosition - from;

	{
		const Vector3 axisX = createVector3(transformMatrix.x.x, transformMatrix.x.y, transformMatrix.x.z);
		const float e = dot(axisX, delta);
		const float f = dot(dir, axisX);

		if (fabs(f) > 0.001f)
		{
			float t1 = (e - halfExtents.x)/f;
			float t2 = (e + halfExtents.x)/f;

			if (t1 > t2) 
			{ 
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}
			if (t1 > tMin)
			{
				tMin = t1;
			}

			if (tMax < tMin)
			{
				return -1.0f;
			}
		}
		else
		{
			if (-e - halfExtents.x > 0.0f || -e + halfExtents.x < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	{
		const Vector3 axisY = createVector3(transformMatrix.y.x, transformMatrix.y.y, transformMatrix.y.z);
		const float e = dot(axisY, delta);
		const float f = dot(dir, axisY);

		if (fabs(f) > 0.001f)
		{
			float t1 = (e- halfExtents.y)/f;
			float t2 = (e+ halfExtents.y)/f;

			if (t1 > t2) 
			{ 
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}
			if (t1 > tMin)
			{
				tMin = t1;
			}
			if (tMin > tMax)
			{
				return -1.0f;
			}
		}
		else
		{
			if (-e - halfExtents.y > 0.0f || -e + halfExtents.y < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	{
		const Vector3 axisZ = createVector3(transformMatrix.z.x, transformMatrix.z.y, transformMatrix.z.z);
		const float e = dot(axisZ, delta);
		const float f = dot(dir, axisZ);

		if (fabs(f) > 0.001f)
		{
			float t1 = ( e- halfExtents.z)/f;
			float t2 = ( e+ halfExtents.z)/f;

			if (t1 > t2) 
			{ 
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}
			if (t1 > tMin)
			{
				tMin = t1;
			}
			if (tMin > tMax)
			{
				return -1.0f;
			}
		}
		else
		{
			if (-e - halfExtents.z > 0.0f || -e + halfExtents.z < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	return tMin;
}

float getRayTriangleIntersection(const Vector3& from, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	const Vector3 verts[] = { v0, v1, v2 };
	const uint16_t inds[] = { 0, 1, 2 };
	return getRayMeshIntersection(from, dir, MATRIX4X4_IDENTITY, verts, sizeof(Vector3), inds, 3);
}

float getRayMeshIntersection(const Vector3& from, const Vector3& dir, const Matrix4x4& transformMatrix, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t num)
{
	bool hit = false;
	float tMin = 999999999.9f;

	for (uint32_t i = 0; i < num; i += 3)
	{
		const uint32_t i0 = indices[i + 0];
		const uint32_t i1 = indices[i + 1];
		const uint32_t i2 = indices[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertices + i0*stride) * transformMatrix;
		const Vector3& v1 = *(const Vector3*)((const char*)vertices + i1*stride) * transformMatrix;
		const Vector3& v2 = *(const Vector3*)((const char*)vertices + i2*stride) * transformMatrix;

		// Find vectors for two edges sharing v0
		const Vector3 e1 = v1 - v0;
		const Vector3 e2 = v2 - v0;

		// Begin calculating determinant - also used to calculate u parameter
		const Vector3 P = cross(dir, e2);

		// If determinant is near zero, ray lies in plane of triangle
		const float det = dot(e1, P);
		if (getAreFloatsEqual(det, 0.0f))
		{
			continue;
		}

		const float inv_det = 1.0f / det;

		// Distance from v0 to ray origin
		const Vector3 T = from - v0;

		// u parameter and test bound
		const float u = dot(T, P) * inv_det;

		// The intersection lies outside of the triangle
		if (u < 0.0f || u > 1.0f)
		{
			continue;
		}

		// Prepare to test v parameter
		const Vector3 Q = cross(T, e1);

		// v parameter and test bound
		const float v = dot(dir, Q) * inv_det;

		// The intersection lies outside of the triangle
		if (v < 0.0f || u + v  > 1.0f)
		{
			continue;
		}

		const float t = dot(e2, Q) * inv_det;

		// Ray intersection
		if (t > FLOAT_EPSILON)
		{
			hit = true;
			tMin = getMinFloat(t, tMin);
		}
	}

	return hit ? tMin : -1.0f;
}

bool getThreePlanesIntersection(const Plane& a, const Plane& b, const Plane& c, Vector3& ip)
{
	const Vector3 na = a.n;
	const Vector3 nb = b.n;
	const Vector3 nc = c.n;
	const float den = -dot(cross(na, nb), nc);

	if (getAreFloatsEqual(den, 0.0f))
	{
		return false;
	}

	const float invDen = 1.0f / den;

	const Vector3 nbnc = a.d * cross(nb, nc);
	const Vector3 ncna = b.d * cross(nc, na);
	const Vector3 nanb = c.d * cross(na, nb);

	ip = (nbnc + ncna + nanb) * invDen;

	return true;
}

bool getFrustumSphereIntersection(const Frustum& f, const Sphere& s)
{
	if (PlaneFn::getDistanceToPoint(f.left, s.c) < -s.r ||
		PlaneFn::getDistanceToPoint(f.right, s.c) < -s.r)
	{
		return false;
	}

	if (PlaneFn::getDistanceToPoint(f.bottom, s.c) < -s.r ||
		PlaneFn::getDistanceToPoint(f.top, s.c) < -s.r)
	{
		return false;
	}

	if (PlaneFn::getDistanceToPoint(f.near, s.c) < -s.r ||
		PlaneFn::getDistanceToPoint(f.far, s.c) < -s.r)
	{
		return false;
	}

	return true;
}

bool getFrustumBoxIntersection(const Frustum& f, const Aabb& b)
{
	const Vector3 v0 = AabbFn::getVertex(b, 0);
	const Vector3 v1 = AabbFn::getVertex(b, 1);
	const Vector3 v2 = AabbFn::getVertex(b, 2);
	const Vector3 v3 = AabbFn::getVertex(b, 3);
	const Vector3 v4 = AabbFn::getVertex(b, 4);
	const Vector3 v5 = AabbFn::getVertex(b, 5);
	const Vector3 v6 = AabbFn::getVertex(b, 6);
	const Vector3 v7 = AabbFn::getVertex(b, 7);

	uint8_t out = 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.left, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.right, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.bottom, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.top, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.near, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v0) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v1) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v2) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v3) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v4) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v5) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v6) < 0.0f) ? 1 : 0;
	out += (PlaneFn::getDistanceToPoint(f.far, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	// If we are here, it is because either the box intersects or it is contained in the frustum
	return true;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka