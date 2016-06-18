// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

struct Vector2
{
	float x;
	float y;
};

struct Vector3
{
	float x;
	float y;
	float z;
};

struct Vector4
{
	float x, y, z, w;
};

// RGBA color
typedef Vector4 Color4;

struct Quaternion
{
	float x, y, z, w;
};

struct Matrix3x3
{
	Vector3 x, y, z;
};

struct Matrix4x4
{
	Vector4 x, y, z, t;
};

struct Aabb
{
	Vector3 min;
	Vector3 max;
};

// Oriented bounding box
struct Obb
{
	// position and orientation of the box
	Matrix4x4 tm;
	// size of the box along the axis
	Vector3 halfExtents;
};

// 3D Plane.
// ax + by + cz + d = 0
// where: d = -dot(n, p)
struct Plane
{
	Vector3 n;
	float d;
};

struct Frustum
{
	Plane left;
	Plane right;
	Plane bottom;
	Plane top;
	Plane near;
	Plane far;
};

struct Sphere
{
	Vector3 c;
	float r;
};

const Vector2 VECTOR2_ZERO  = { 0.0f, 0.0f };
const Vector2 VECTOR2_ONE   = { 1.0f, 1.0f };
const Vector2 VECTOR2_XAXIS = { 1.0f, 0.0f };
const Vector2 VECTOR2_YAXIS = { 0.0f, 1.0f };

const Vector3 VECTOR3_ZERO     = {  0.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_ONE      = {  1.0f,  1.0f,  1.0f };
const Vector3 VECTOR3_XAXIS    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_YAXIS    = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_ZAXIS    = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_RIGHT    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_LEFT     = { -1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_UP       = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_DOWN     = {  0.0f, -1.0f,  0.0f };
const Vector3 VECTOR3_FORWARD  = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_BACKWARD = {  0.0f,  0.0f, -1.0f };

const Vector4 VECTOR4_ZERO  = { 0.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_ONE   = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vector4 VECTOR4_XAXIS = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_YAXIS = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_ZAXIS = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vector4 VECTOR4_WAXIS = { 0.0f, 0.0f, 0.0f, 1.0f };

const Color4 COLOR4_BLACK  = { 0.0f, 0.0f, 0.0f, 1.0f };
const Color4 COLOR4_WHITE  = { 1.0f, 1.0f, 1.0f, 1.0f };
const Color4 COLOR4_RED    = { 1.0f, 0.0f, 0.0f, 1.0f };
const Color4 COLOR4_GREEN  = { 0.0f, 1.0f, 0.0f, 1.0f };
const Color4 COLOR4_BLUE   = { 0.0f, 0.0f, 1.0f, 1.0f };
const Color4 COLOR4_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
const Color4 COLOR4_ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f };

const Quaternion QUATERNION_IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

const Matrix3x3 MATRIX3X3_IDENTITY = { VECTOR3_XAXIS, VECTOR3_YAXIS, VECTOR3_ZAXIS };

const Matrix4x4 MATRIX4X4_IDENTITY = { VECTOR4_XAXIS, VECTOR4_YAXIS, VECTOR4_ZAXIS, VECTOR4_WAXIS };

const Plane PLANE_ZERO  = { VECTOR3_ZERO,  0.0f };
const Plane PLANE_XAXIS = { VECTOR3_XAXIS, 0.0f };
const Plane PLANE_YAXIS = { VECTOR3_YAXIS, 0.0f };
const Plane PLANE_ZAXIS = { VECTOR3_ZAXIS, 0.0f };

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
