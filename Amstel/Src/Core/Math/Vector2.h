// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"

namespace Rio
{

inline Vector2 createVector2(float x, float y)
{
	Vector2 v;
	v.x = x;
	v.y = y;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector2& operator+=(Vector2& a, const Vector2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector2& operator-=(Vector2& a, const Vector2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2& operator*=(Vector2& a, float k)
{
	a.x *= k;
	a.y *= k;
	return a;
}

// Negates <a> and returns the result
inline Vector2 operator-(const Vector2& a)
{
	Vector2 v;
	v.x = -a.x;
	v.y = -a.y;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector2 operator+(Vector2 a, const Vector2& b)
{
	a += b;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector2 operator-(Vector2 a, const Vector2& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2 operator*(Vector2 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2 operator*(float k, Vector2 a)
{
	a *= k;
	return a;
}

// Returns true whether the vectors <a> and <b> are equal
inline bool operator==(const Vector2& a, const Vector2& b)
{
	return getAreFloatsEqual(a.x, b.x) && getAreFloatsEqual(a.y, b.y);
}

// Returns the dot product between the vectors <a> and <b>
inline float dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

// Returns the squared length of <a>
inline float getLengthSquared(const Vector2& a)
{
	return dot(a, a);
}

// Returns the length of <a>
inline float getLength(const Vector2& a)
{
	return sqrtf(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector2 normalize(Vector2& a)
{
	const float length = getLength(a);
	const float invertedLength = 1.0f / length;
	a.x *= invertedLength;
	a.y *= invertedLength;
	return a;
}

// Sets the length of <a> to <length>
inline void setLength(Vector2& a, float length)
{
	normalize(a);
	a.x *= length;
	a.y *= length;
}

// Returns the squared distance between the points <a> and <b>
inline float getDistanceSquared(const Vector2& a, const Vector2& b)
{
	return getLengthSquared(b - a);
}

// Returns the distance between the points <a> and <b>
inline float getDistance(const Vector2& a, const Vector2& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector2& a, const Vector2& b)
{
	return acos(dot(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector2 max(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector2 min(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in [0, 1]
inline Vector2 getLinearInterpolation(const Vector2& a, const Vector2& b, float t)
{
	Vector2 v;
	v.x = getLinearInterpolation(a.x, b.x, t);
	v.y = getLinearInterpolation(a.y, b.y, t);
	return v;
}

// Returns the pointer to the data
inline float* getFloatPointer(Vector2& a)
{
	return &a.x;
}

// Returns the pointer to the data
inline const float* getFloatPointer(const Vector2& a)
{
	return &a.x;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka