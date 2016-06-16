// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

// Pseudo-random number generator
// Uses LCG algorithm: fast and compatible with the standard C rand()
class Random
{
public:
	// Initializes the generator with the given <seed>
	Random(int32_t seed);
	// Returns a pseudo-random integer in the range [0, 32767]
	int32_t getTnteger();
	// Returns a pseudo-random integer in the range [0, max)
	int32_t getTnteger(int32_t max);
	// Returns a pseudo-random float in the range [0.0, 1.0]
	float getUnitFloat();
private:
	int32_t seed;
};

inline Random::Random(int32_t seed)
	: seed(seed)
{
}

inline int32_t Random::getTnteger()
{
	seed = 214013 * seed + 13737667;
	return (seed >> 16) & 0x7fff;
}

inline int32_t Random::getTnteger(int32_t max)
{
	return (max == 0) ? 0 : getTnteger() % max;
}

inline float Random::getUnitFloat()
{
	return getTnteger() / (float)0x7fff;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka