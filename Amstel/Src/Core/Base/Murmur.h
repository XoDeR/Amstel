// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

	uint32_t getMurmurHash32(const void* key, uint32_t length, uint32_t seed);
	uint64_t getMurmurHash64(const void* key, uint32_t length, uint64_t seed);

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
