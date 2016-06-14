// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Thread/Mutex.h"
#include "Core/Base/Platform.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#include "Device/Windows/Headers_Windows.h"
	#include <limits.h>
#endif

namespace Rio
{

struct Semaphore
{
	Semaphore()
	{
#if RIO_PLATFORM_POSIX
		int err = pthread_cond_init(&condition, NULL);
		RIO_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		semaphoreHandle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		RIO_ASSERT(semaphoreHandle != NULL, "CreateSemaphore: GetLastError = %d", GetLastError());
		RIO_UNUSED(semaphoreHandle);
#endif // RIO_PLATFORM_
	}

	~Semaphore()
	{
#if RIO_PLATFORM_POSIX
		int err = pthread_cond_destroy(&condition);
		RIO_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = CloseHandle(semaphoreHandle);
		RIO_ASSERT(err != 0, "CloseHandle: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

	void post(uint32_t count = 1)
	{
#if RIO_PLATFORM_POSIX
		ScopedMutex sm(mutex);

		for (uint32_t i = 0; i < count; ++i)
		{
			int err = pthread_cond_signal(&condition);
			RIO_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
			RIO_UNUSED(err);
		}

		this->count += count;
#elif RIO_PLATFORM_WINDOWS
		BOOL err = ReleaseSemaphore(semaphoreHandle, count, NULL);
		RIO_ASSERT(err != 0, "ReleaseSemaphore: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

	void wait()
	{
#if RIO_PLATFORM_POSIX
		ScopedMutex sm(mutex);

		while (this->count <= 0)
		{
			int err = pthread_cond_wait(&condition, &(mutex.mutex));
			RIO_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
			RIO_UNUSED(err);
		}

		this->count--;
#elif RIO_PLATFORM_WINDOWS
		DWORD err = WaitForSingleObject(semaphoreHandle, INFINITE);
		RIO_ASSERT(err == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

private:
	// Disable copying
	Semaphore(const Semaphore& s) = delete;
	Semaphore& operator=(const Semaphore& s) = delete;
public:
#if RIO_PLATFORM_POSIX
	Mutex mutex;
	pthread_cond_t condition;
	int32_t count = 0;
#elif RIO_PLATFORM_WINDOWS
	HANDLE semaphoreHandle = INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka