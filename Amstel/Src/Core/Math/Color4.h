// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"

namespace Rio
{

inline Color4 createColor4(float r, float g, float b, float a)
{
	Color4 c;
	c.x = r;
	c.y = g;
	c.z = b;
	c.w = a;
	return c;
}

// Alpha is set to 255
inline Color4 createColorRgb(uint8_t r, uint8_t g, uint8_t b)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f;
	return c;
}

inline Color4 createColorRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f/255.0f * a;
	return c;
}

// Returns a new color from packed RGBA integer
inline Color4 createColorRgba(uint32_t rgba)
{
	Color4 c;
	c.x = 1.0f/255.0f * ((rgba & 0xff000000) >> 24);
	c.y = 1.0f/255.0f * ((rgba & 0x00ff0000) >> 16);
	c.z = 1.0f/255.0f * ((rgba & 0x0000ff00) >> 8);
	c.w = 1.0f/255.0f * ((rgba & 0x000000ff) >> 0);
	return c;
}

// Returns the color as a packed RGBA integer; alpha is set to 255
inline uint32_t getRgb(const Color4& c)
{
	uint32_t rgba;
	rgba =	(uint32_t)(255.0f * c.x) << 24;
	rgba |= (uint32_t)(255.0f * c.y) << 16;
	rgba |= (uint32_t)(255.0f * c.z) << 8;
	rgba |= 255;
	return rgba;
}

// Returns the color as a packed ABGR integer; alpha is set to 255
inline uint32_t getBgr(const Color4& c)
{
	uint32_t abgr;
	abgr =	255 << 24;
	abgr |= (uint32_t)(255.0f * c.z) << 16;
	abgr |= (uint32_t)(255.0f * c.y) << 8;
	abgr |= (uint32_t)(255.0f * c.x);
	return abgr;
}

// Returns the color as a packed 32-bit integer
// RGBA order
inline uint32_t getRgba(const Color4& c)
{
	uint32_t rgba;
	rgba =	(uint32_t)(255.0f * c.x) << 24;
	rgba |= (uint32_t)(255.0f * c.y) << 16;
	rgba |= (uint32_t)(255.0f * c.z) << 8;
	rgba |= (uint32_t)(255.0f * c.w);
	return rgba;
}

// Returns the color as a packed 32-bit integer
// ABGR order
inline uint32_t getAbgr(const Color4& c)
{
	uint32_t abgr;
	abgr =	(uint32_t)(255.0f * c.w) << 24;
	abgr |= (uint32_t)(255.0f * c.z) << 16;
	abgr |= (uint32_t)(255.0f * c.y) << 8;
	abgr |= (uint32_t)(255.0f * c.x);
	return abgr;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka