// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include <stdint.h> // uint32_t, etc

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif // _MSC_VER

#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif // alignof

// Copyright (c) 2016 Volodymyr Syvochka
