#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"

namespace Rio
{

inline Vector3 createVector3(float x, float y, float z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

inline Vector3& operator+=(Vector3& a, const Vector3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline Vector3& operator*=(Vector3& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	return a;
}

inline Vector3 operator-(const Vector3& a)
{
	Vector3 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	return v;
}

inline Vector3 operator+(Vector3 a, const Vector3& b)
{
	a += b;
	return a;
}

inline Vector3 operator-(Vector3 a, const Vector3& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector3 operator*(Vector3 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector3 operator*(float k, Vector3 a)
{
	a *= k;
	return a;
}

// Returns true whether the vectors <a> and <b> are equal
inline bool operator==(const Vector3& a, const Vector3& b)
{
	return getAreFloatsEqual(a.x, b.x) && getAreFloatsEqual(a.y, b.y) && getAreFloatsEqual(a.z, b.z);
}

// Returns the dot product between the vectors <a> and <b>
inline float dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Returns the cross product between the vectors <a> and <b>
inline Vector3 cross(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}

// Returns the squared length of <a>
inline float getLengthSquared(const Vector3& a)
{
	return dot(a, a);
}

inline float getLength(const Vector3& a)
{
	return sqrtf(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector3 normalize(Vector3& a)
{
	const float length = getLength(a);
	const float invertedLength = 1.0f / length;
	a.x *= invertedLength;
	a.y *= invertedLength;
	a.z *= invertedLength;
	return a;
}

// Sets the length of <a> to <length>
inline void setLength(Vector3& a, float length)
{
	normalize(a);
	a.x *= length;
	a.y *= length;
	a.z *= length;
}

// Returns the squared distance between the points <a> and <b>
inline float getDistanceSquared(const Vector3& a, const Vector3& b)
{
	return getLengthSquared(b - a);
}

// Returns the distance between the points <a> and <b>
inline float getDistance(const Vector3& a, const Vector3& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector3& a, const Vector3& b)
{
	return acos(dot(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector3 max(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	v.z = getMaxFloat(a.z, b.z);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector3 min(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	v.z = getMinFloat(a.z, b.z);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in [0, 1]
inline Vector3 getLinearInterpolation(const Vector3& a, const Vector3& b, float t)
{
	Vector3 v;
	v.x = getLinearInterpolation(a.x, b.x, t);
	v.y = getLinearInterpolation(a.y, b.y, t);
	v.z = getLinearInterpolation(a.z, b.z, t);
	return v;
}

// Returns the pointer to the data
inline float* getFloatPointer(Vector3& a)
{
	return &a.x;
}

// Returns the pointer to the data
inline const float* getFloatPointer(const Vector3& a)
{
	return &a.x;
}

// Returns the Vector2 portion of <a> (truncates z)
inline Vector2 getVector2(const Vector3& a)
{
	Vector2 v;
	v.x = a.x;
	v.y = a.y;
	return v;
}

} // namespace Rio
