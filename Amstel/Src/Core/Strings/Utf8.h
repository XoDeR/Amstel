// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

namespace Rio
{

namespace Utf8Fn
{
	uint32_t decode(uint32_t* state, uint32_t* codep, uint32_t byte);

} // namespace Utf8Fn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka