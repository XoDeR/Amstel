// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"

namespace Rio
{

// Returns a new vector from individual elements
inline Vector4 createVector4(float x, float y, float z, float w)
{
	Vector4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

// Adds 2 vectors and returns the result
inline Vector4& operator+=(Vector4& a,	const Vector4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector4& operator-=(Vector4& a,	const Vector4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4& operator*=(Vector4& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.w *= k;
	return a;
}

// Negates <a> and returns the result
inline Vector4 operator-(const Vector4& a)
{
	Vector4 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	v.w = -a.w;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector4 operator+(Vector4 a, const Vector4& b)
{
	a += b;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector4 operator-(Vector4 a, const Vector4& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4 operator*(Vector4 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4 operator*(float k, Vector4 a)
{
	a *= k;
	return a;
}

inline bool operator==(const Vector4& a, const Vector4& b)
{
	return getAreFloatsEqual(a.x, b.x)
		&& getAreFloatsEqual(a.y, b.y)
		&& getAreFloatsEqual(a.z, b.z)
		&& getAreFloatsEqual(a.w, b.w)
		;
}

inline float dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float getLengthSquared(const Vector4& a)
{
	return dot(a, a);
}

inline float getLength(const Vector4& a)
{
	return sqrtf(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector4 normalize(Vector4& a)
{
	const float length = getLength(a);
	const float invertedLength = 1.0f / length;
	a.x *= invertedLength;
	a.y *= invertedLength;
	a.z *= invertedLength;
	a.w *= invertedLength;
	return a;
}

// Sets the length of <a> to <length>
inline void setLength(Vector4& a, float length)
{
	normalize(a);
	a.x *= length;
	a.y *= length;
	a.z *= length;
	a.w *= length;
}

inline float getDistanceSquared(const Vector4& a, const Vector4& b)
{
	return getLengthSquared(b - a);
}

inline float getDistance(const Vector4& a, const Vector4& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector4& a, const Vector4& b)
{
	return acos(dot(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector4 max(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	v.z = getMaxFloat(a.z, b.z);
	v.w = getMaxFloat(a.w, b.w);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector4 min(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	v.z = getMinFloat(a.z, b.z);
	v.w = getMinFloat(a.w, b.w);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in [0, 1]
inline Vector4 getLinearInterpolation(const Vector4& a, const Vector4& b, float t)
{
	Vector4 v;
	v.x = getLinearInterpolation(a.x, b.x, t);
	v.y = getLinearInterpolation(a.y, b.y, t);
	v.z = getLinearInterpolation(a.z, b.z, t);
	v.w = getLinearInterpolation(a.w, b.w, t);
	return v;
}

// Returns the pointer to the data
inline float* getFloatPointer(Vector4& a)
{
	return &a.x;
}

// Returns the pointer to the data
inline const float* getFloatPointer(const Vector4& a)
{
	return &a.x;
}

// Returns the Vector3 portion of <a> (truncates w)
inline Vector3 getVector3(const Vector4& a)
{
	Vector3 v;
	v.x = a.x;
	v.y = a.y;
	v.z = a.z;
	return v;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka