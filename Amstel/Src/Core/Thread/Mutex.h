// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Base/Macros.h"
#include "Core/Base/Platform.h"
#include "Core/Base/Types.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#include "Device/Windows/Headers_Windows.h"
#endif // RIO_PLATFORM_

namespace Rio
{

struct Mutex
{
	Mutex()
	{
#if RIO_PLATFORM_POSIX
		pthread_mutexattr_t attr;
		int err = pthread_mutexattr_init(&attr);
		RIO_ASSERT(err == 0, "pthread_mutexattr_init: errno = %d", err);
		err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		RIO_ASSERT(err == 0, "pthread_mutexattr_settype: errno = %d", err);
		err = pthread_mutex_init(&mutex, &attr);
		RIO_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
		err = pthread_mutexattr_destroy(&attr);
		RIO_ASSERT(err == 0, "pthread_mutexattr_destroy: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		InitializeCriticalSection(&criticalSection);
#endif // RIO_PLATFORM_
	}

	~Mutex()
	{
#if RIO_PLATFORM_POSIX
		int err = pthread_mutex_destroy(&mutex);
		RIO_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		DeleteCriticalSection(&criticalSection);
#endif // RIO_PLATFORM_
	}

	void lock()
	{
#if RIO_PLATFORM_POSIX
		int err = pthread_mutex_lock(&mutex);
		RIO_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		EnterCriticalSection(&criticalSection);
#endif // RIO_PLATFORM_
	}

	void unlock()
	{
#if RIO_PLATFORM_POSIX
		int err = pthread_mutex_unlock(&mutex);
		RIO_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		LeaveCriticalSection(&criticalSection);
#endif // RIO_PLATFORM_
	}

private:
	// Disable copying.
	Mutex(const Mutex&) = delete;
	Mutex& operator=(const Mutex&) = delete;
public:
#if RIO_PLATFORM_POSIX
	pthread_mutex_t mutex;
#elif RIO_PLATFORM_WINDOWS
	CRITICAL_SECTION criticalSection;
#endif // RIO_PLATFORM_

};

// Automatically locks a mutex when created and unlocks when destroyed
// RAII wrapper
struct ScopedMutex
{
	// Locks the mutex
	ScopedMutex(Mutex& m)
		: mutex(m)
	{
		mutex.lock();
	}

	// Unlocks the mutex
	~ScopedMutex()
	{
		mutex.unlock();
	}
	
private:
	// Disable copying
	ScopedMutex(const ScopedMutex&) = delete;
	ScopedMutex& operator=(const ScopedMutex&) = delete;
public:
	Mutex& mutex;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka