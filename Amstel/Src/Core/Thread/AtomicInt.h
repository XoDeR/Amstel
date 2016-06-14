// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Platform.h"

#if RIO_PLATFORM_WINDOWS
	#include "Core/Base/Types.h"
	#include "Device/Windows/Headers_Windows.h"
#endif

namespace Rio
{

struct AtomicInt
{
	AtomicInt(int val)
	{
		store(val);
	}

	int load() const
	{
#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
		__sync_fetch_and_add(&atomicValue, 0);
		return atomicValue;
#elif RIO_PLATFORM_WINDOWS
		InterlockedExchangeAdd(&atomicValue, (int32_t)0);
		return atomicValue;
#endif // RIO_PLATFORM_
	}

	void store(int val)
	{
#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
		__sync_lock_test_and_set(&atomicValue, val);
#elif RIO_PLATFORM_WINDOWS
		InterlockedExchange(&atomicValue, val);
#endif // RIO_PLATFORM_
	}

#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
	mutable int atomicValue;
#elif RIO_PLATFORM_WINDOWS
	mutable LONG atomicValue;
#endif // RIO_PLATFORM_
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka