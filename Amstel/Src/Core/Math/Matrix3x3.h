// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

// Returns a new matrix from individual elements
inline Matrix3x3 createMatrix3x3(float xx, float xy, float xz, float yx, float yy, float yz, float zx, float zy, float zz)
{
	Matrix3x3 m;
	m.x.x = xx;
	m.x.y = xy;
	m.x.z = xz;

	m.y.x = yx;
	m.y.y = yy;
	m.y.z = yz;

	m.z.x = zx;
	m.z.y = zy;
	m.z.z = zz;
	return m;
}

// Returns a new matrix from axes <x>, <y> and <z>
inline Matrix3x3 createMatrix3x3(const Vector3& x, const Vector3& y, const Vector3& z)
{
	Matrix3x3 m;
	m.x = x;
	m.y = y;
	m.z = z;
	return m;
}

// Returns a new matrix from rotation <r>
inline Matrix3x3 createMatrix3x3(const Quaternion& r)
{
	Matrix3x3 m;
	m.x.x = 1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z;
	m.x.y = 2.0f * r.x * r.y + 2.0f * r.w * r.z;
	m.x.z = 2.0f * r.x * r.z - 2.0f * r.w * r.y;

	m.y.x = 2.0f * r.x * r.y - 2.0f * r.w * r.z;
	m.y.y = 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z;
	m.y.z = 2.0f * r.y * r.z + 2.0f * r.w * r.x;

	m.z.x = 2.0f * r.x * r.z + 2.0f * r.w * r.y;
	m.z.y = 2.0f * r.y * r.z - 2.0f * r.w * r.x;
	m.z.z = 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y;
	return m;
}

// Adds the matrix <a> to <b> and returns the result
inline Matrix3x3& operator+=(Matrix3x3& a, const Matrix3x3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

// Subtracts the matrix <b> from <a> and returns the result
inline Matrix3x3& operator-=(Matrix3x3& a, const Matrix3x3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix3x3& operator*=(Matrix3x3& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	return a;
}

// Multiplies the matrix <a> by <b> and returns the result (transforms first by <a> then by <b>)
inline Matrix3x3& operator*=(Matrix3x3& a, const Matrix3x3& b)
{
	Matrix3x3 tmp;
	tmp.x.x = a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x;
	tmp.x.y = a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y;
	tmp.x.z = a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z;

	tmp.y.x = a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x;
	tmp.y.y = a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y;
	tmp.y.z = a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z;

	tmp.z.x = a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x;
	tmp.z.y = a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y;
	tmp.z.z = a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z;

	a = tmp;
	return a;
}

// Adds the matrix <a> to <b> and returns the result
inline Matrix3x3 operator+(Matrix3x3 a, const Matrix3x3& b)
{
	a += b;
	return a;
}

// Subtracts the matrix <b> from <a> and returns the result
inline Matrix3x3 operator-(Matrix3x3 a, const Matrix3x3& b)
{
	a -= b;
	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix3x3 operator*(Matrix3x3 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix3x3 operator*(float k, Matrix3x3 a)
{
	a *= k;
	return a;
}

// Multiplies the matrix <a> by the vector <v> and returns the result
inline Vector3 operator*(const Vector3& v, const Matrix3x3& a)
{
	Vector3 r;
	r.x = v.x*a.x.x + v.y*a.y.x + v.z*a.z.x;
	r.y = v.x*a.x.y + v.y*a.y.y + v.z*a.z.y;
	r.z = v.x*a.x.z + v.y*a.y.z + v.z*a.z.z;
	return r;
}

// Multiplies the matrix <a> by <b> and returns the result (transforms first by <a> then by <b>)
inline Matrix3x3 operator*(Matrix3x3 a, const Matrix3x3& b)
{
	a *= b;
	return a;
}

// Transposes the matrix <m> and returns the result
inline Matrix3x3& transpose(Matrix3x3& m)
{
	float tmp;

	tmp = m.x.y;
	m.x.y = m.y.x;
	m.y.x = tmp;

	tmp = m.x.z;
	m.x.z = m.z.x;
	m.z.x = tmp;

	tmp = m.y.z;
	m.y.z = m.z.y;
	m.z.y = tmp;

	return m;
}

// Returns the transposed of the matrix <m>
inline Matrix3x3 getTransposed(Matrix3x3 m)
{
	transpose(m);
	return m;
}

// Returns the determinant of the matrix <m>
inline float getDeterminant(const Matrix3x3& m)
{
	return	+ m.x.x * (m.y.y * m.z.z - m.z.y * m.y.z)
			- m.y.x * (m.x.y * m.z.z - m.z.y * m.x.z)
			+ m.z.x * (m.x.y * m.y.z - m.y.y * m.x.z)
			;
}

// Inverts the matrix <m> and returns the result
inline Matrix3x3& invert(Matrix3x3& m)
{
	const float xx = m.x.x;
	const float xy = m.x.y;
	const float xz = m.x.z;
	const float yx = m.y.x;
	const float yy = m.y.y;
	const float yz = m.y.z;
	const float zx = m.z.x;
	const float zy = m.z.y;
	const float zz = m.z.z;

	const float det = getDeterminant(m);
	const float inv_det = 1.0f / det;

	m.x.x = + (yy*zz - zy*yz) * inv_det;
	m.x.y = - (xy*zz - zy*xz) * inv_det;
	m.x.z = + (xy*yz - yy*xz) * inv_det;

	m.y.x = - (yx*zz - zx*yz) * inv_det;
	m.y.y = + (xx*zz - zx*xz) * inv_det;
	m.y.z = - (xx*yz - yx*xz) * inv_det;

	m.z.x = + (yx*zy - zx*yy) * inv_det;
	m.z.y = - (xx*zy - zx*xy) * inv_det;
	m.z.z = + (xx*yy - yx*xy) * inv_det;

	return m;
}

// Returns the inverse of the matrix <m>
inline Matrix3x3 getInverted(Matrix3x3 m)
{
	invert(m);
	return m;
}

// Sets the matrix <m> to identity
inline void setToIdentity(Matrix3x3& m)
{
	m.x.x = 1.0f;
	m.x.y = 0.0f;
	m.x.z = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 1.0f;
	m.y.z = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f;
}

// Returns the scale of the matrix <m>
inline Vector3 getScale(const Matrix3x3& m)
{
	const float sx = getLength(m.x);
	const float sy = getLength(m.y);
	const float sz = getLength(m.z);
	Vector3 v;
	v.x = sx;
	v.y = sy;
	v.z = sz;
	return v;
}

// Sets the scale of the matrix <m>
inline void setScale(Matrix3x3& m, const Vector3& s)
{
	setLength(m.x, s.x);
	setLength(m.y, s.y);
	setLength(m.z, s.z);
}

// Returns the pointer to the matrix's data
inline float* getFloatPointer(Matrix3x3& m)
{
	return &m.x.x;
}

// Returns the pointer to the matrix's data
inline const float* getFloatPointer(const Matrix3x3& m)
{
	return &m.x.x;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka