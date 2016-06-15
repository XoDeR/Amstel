// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include <stddef.h> // NULL
#include <stdint.h> // uint32_t, etc

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif

#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

// Copyright (c) 2016 Volodymyr Syvochka
