// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"

namespace Rio
{

// Returns the distance along ray (from, dir) to intersection point with plane <p>
// or -1.0 if no intersection
float getRayPlaneIntersection(const Vector3& from, const Vector3& dir, const Plane& p);

// Returns the distance along ray (from, dir) to intersection point with disc defined by
// <center>, <radius> and <normal> or -1.0 if no intersection
float getRayDiscIntersection(const Vector3& from, const Vector3& dir, const Vector3& center, float radius, const Vector3& normal);

// Returns the distance along ray (from, dir) to intersection point with sphere <s>
// or -1.0 if no intersection
float getRaySphereIntersection(const Vector3& from, const Vector3& dir, const Sphere& s);

// Returns the distance along ray (from, dir) to intersection point with the oriented
// bounding box (transformMatrix, halfExtents) or -1.0 if no intersection
float getRayObbIntersection(const Vector3& from, const Vector3& dir, const Matrix4x4& transformMatrix, const Vector3& halfExtents);

// Returns the distance along ray (from, dir) to intersection point with the triangle
// (v0, v1, v2) or -1.0 if no intersection
float getRayTriangleIntersection(const Vector3& from, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2);

// Returns the distance along ray (from, dir) to intersection point with the triangle
// mesh defined by (vertices, stride, indices, num) or -1.0 if no intersection
float getRayMeshIntersection(const Vector3& from, const Vector3& dir, const Matrix4x4& transformMatrix, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t num);

// Returns whether the planes <a>, <b> and <c> intersects and if so fills <ip> with the intersection point
bool getThreePlanesIntersection(const Plane& a, const Plane& b, const Plane& c, Vector3& ip);

// Returns whether the frustum <f> and the sphere <s> intersects
bool getFrustumSphereIntersection(const Frustum& f, const Sphere& s);

// Returns whether the frustum <f> and the Aabb <b> intersects
bool getFrustumBoxIntersection(const Frustum& f, const Aabb& b);

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka