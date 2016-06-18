// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include <math.h> // acos; sqrtf; sinf; cosf; tanf; floorf

namespace Rio
{

const float PI = 3.14159265358979323846f;
const float PI_TWO = 6.28318530717958647693f;
const float PI_HALF = 1.57079632679489661923f;
const float FLOAT_EPSILON = 1.0e-7f;

// Returns whether <a> and <b> are equal according to <epsilon>
inline bool getAreFloatsEqual(float a, float b, float epsilon = FLOAT_EPSILON)
{
	return b <= (a + epsilon)
		&& b >= (a - epsilon)
		;
}

// Returns the minimum of <a> and <b>
inline float getMinFloat(float a, float b)
{
	return a < b ? a : b;
}

// Returns the maximum of <a> and <b>
inline float getMaxFloat(float a, float b)
{
	return a < b ? b : a;
}

// Clamps <val> to <min> and <max>
inline float getClampedFloat(float min, float max, float val)
{
	return getMinFloat(getMaxFloat(min, val), max);
}

// Returns the fractional part
inline float getFractionalFloat(float a)
{
	return a - floorf(a);
}

// Returns <degrees> in radians
inline float getRadiansFromDegrees(float degrees)
{
	return degrees * PI / 180.0f;
}

// Returns <radians> in degrees
inline float getDegreesFromRadians(float radians)
{
	return radians * 180.0f / PI;
}

// Returns the linear interpolated value between <p0> and <p1> at time <t>
inline float getLinearInterpolation(const float p0, const float p1, float t)
{
	return (1.0f - t) * p0 + t * p1;
}

// Returns the cosine interpolated value between <p0> and <p1> at time <t>
inline float getCosineInterpolation(const float p0, const float p1, float t)
{
	const float f = t * PI;
	const float g = (1.0f - cosf(f)) * 0.5f;

	return p0 + g * (p1 - p0);
}

// Returns the cubic interpolated value between <p0> and <p1> at time <t>
inline float getCubicInterpolation(const float p0, const float p1, float t)
{
	const float tt  = t * t;
	const float ttt = tt * t;

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt  - 2.0f * ttt);
}

// Bezier interpolation
inline float getBezierInterpolation(const float p0, const float p1, const float p2, const float p3, float t)
{
	const float u   = 1.0f - t;
	const float tt  = t * t ;
	const float uu  = u * u;
	const float uuu = uu * u;
	const float ttt = tt * t;

	const float tmp = (uuu * p0)
		+ (3.0f * uu * t * p1)
		+ (3.0f * u * tt * p2)
		+ (ttt * p3);

	return tmp;
}

// Catmull-Rom interpolation
inline float getCatmullRomInterpolation(const float p0, const float p1, const float p2, const float p3, float t)
{
	const float tt  = t * t;
	const float ttt = tt * t;

	const float tmp = (2.0f * p1)
		+ (-p0 + p2) * t
		+ ((2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3) * tt
		+ (-p0 + (3.0f * p1) + (-3.0f * p2) + p3) * ttt;

	return tmp * 0.5f;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka