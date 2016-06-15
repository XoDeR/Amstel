// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

// Optimized power of two
inline uint32_t getNextPowerOf2(uint32_t x)
{
	x--;

	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;

	return ++x;
}

// Optimized power of two
inline bool getIsPowerOfTwo(uint32_t x)
{
	return !(x & (x - 1)) && x;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka