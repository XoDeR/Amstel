// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Platform.h"

#if RIO_COMPILER_GCC
	#define RIO_EXPORT __attribute__ ((visibility("default")))
	#define RIO_INLINE inline
	#define RIO_THREAD __thread
#elif RIO_COMPILER_MSVC
	#define RIO_EXPORT __declspec(dllexport)
	#define RIO_INLINE __inline
	#define RIO_THREAD __declspec(thread)
#else
	#error "Compiler not supported"
#endif

#define RIO_UNUSED(x) do { (void)(x); } while (0)
#define RIO_COUNTOF(arr) (sizeof(arr)/sizeof(arr[0]))
#define RIO_CONCATENATE1(a, b) a ## b
#define RIO_CONCATENATE(a, b) RIO_CONCATENATE1(a, b)
#define RIO_STATIC_ASSERT(condition) typedef int RIO_CONCATENATE(STATIC_ASSERT,__LINE__)[condition ? 1 : -1]
// Copyright (c) 2016 Volodymyr Syvochka