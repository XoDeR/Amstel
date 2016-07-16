// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Matrix3x3.h"

namespace Rio
{

inline Quaternion createQuaternion(float x, float y, float z, float w)
{
	Quaternion q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

inline Quaternion createQuaternion(const Vector3& axis, float angle)
{
	const float ha = angle * 0.5f;
	const float sa = sinf(ha);
	const float ca = cosf(ha);
	Quaternion q;
	q.x = axis.x * sa;
	q.y = axis.y * sa;
	q.z = axis.z * sa;
	q.w = ca;
	return q;
}

// Returns a new quaternion from matrix
Quaternion createQuaternion(const Matrix3x3& m);

inline Quaternion& operator*=(Quaternion& a, const Quaternion& b)
{
	const float tx = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	const float ty = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	const float tz = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	const float tw = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	a.x = tx;
	a.y = ty;
	a.z = tz;
	a.w = tw;
	return a;
}

// Negates the quaternion and returns the result
inline Quaternion operator-(const Quaternion& q)
{
	Quaternion r;
	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = -q.w;
	return r;
}

// Multiplies the quaternions <a> and <b> (rotates first by <a> then by <b>)
inline Quaternion operator*(Quaternion a, const Quaternion& b)
{
	a *= b;
	return a;
}

inline Quaternion operator*(const Quaternion& q, float k)
{
	Quaternion r;
	r.x = q.x * k;
	r.y = q.y * k;
	r.z = q.z * k;
	r.w = q.w * k;
	return r;
}

inline float dot(const Quaternion& a, const Quaternion& b)
{
	return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float getLength(const Quaternion& q)
{
	return sqrtf(dot(q, q));
}

// Normalizes the quaternion <q> and returns the result
inline Quaternion& normalize(Quaternion& q)
{
	const float length = getLength(q);
	const float invertedLength = 1.0f / length;
	q.x *= invertedLength;
	q.y *= invertedLength;
	q.z *= invertedLength;
	q.w *= invertedLength;
	return q;
}

// Returns the conjugate of quaternion
inline Quaternion getConjugate(const Quaternion& q)
{
	Quaternion r;
	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = q.w;
	return r;
}

// Returns the inverse of quaternion
inline Quaternion getInverse(const Quaternion& q)
{
	const float length = getLength(q);
	const float invertedLength = 1.0f / length;
	return getConjugate(q) * invertedLength;
}

// Returns the quaternion <q> raised to the power of <exp>
inline Quaternion getPower(const Quaternion& q, float exp)
{
	if (fabs(q.w) < 0.9999)
	{
		const float alpha = acos(q.w); // alpha = theta/2
		const float newAlpha = alpha * exp;
		const float mult = sinf(newAlpha) / sinf(alpha);

		Quaternion r;
		r.w = cosf(newAlpha);
		r.x = q.x * mult;
		r.y = q.y * mult;
		r.z = q.z * mult;
		return r;
	}

	return q;
}

// Returns the quaternion describing the rotation needed to face towards <direction>
inline Quaternion getLook(const Vector3& direction, const Vector3& up = VECTOR3_YAXIS)
{
	const Vector3 right = cross(direction, up);
	const Vector3 nup = cross(right, direction);

	Matrix3x3 m;
	m.x = -right;
	m.y = nup;
	m.z = direction;
	return createQuaternion(m);
}

// Returns the right axis of the rotation represented by <q>
inline Vector3 getRight(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.x;
}

// Returns the up axis of the rotation represented by <q>
inline Vector3 getUp(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.y;
}

// Returns the forward axis of the rotation represented by <q>
inline Vector3 getForward(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.z;
}

// Returns the linearly interpolated quaternion between *a* and *b* at time *t* in [0, 1]. It uses NLerp.
inline Quaternion getLinearInterpolation(const Quaternion& a, const Quaternion& b, float t)
{
	const float t1 = 1.0f - t;

	Quaternion r;

	if (dot(a, b) < 0.0f)
	{
		r.x = t1*a.x + t*-b.x;
		r.y = t1*a.y + t*-b.y;
		r.z = t1*a.z + t*-b.z;
		r.w = t1*a.w + t*-b.w;
	}
	else
	{
		r.x = t1*a.x + t*b.x;
		r.y = t1*a.y + t*b.y;
		r.z = t1*a.z + t*b.z;
		r.w = t1*a.w + t*b.w;
	}

	return normalize(r);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka