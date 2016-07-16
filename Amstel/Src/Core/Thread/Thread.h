// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Base/Platform.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Base/Types.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#include "Device/Windows/Headers_Windows.h"
	#include <process.h>
#endif

namespace Rio
{

struct Thread
{
	using ThreadFunction = int32_t (*)(void* data);

	Thread()
	{
	}

	~Thread()
	{
		if (isRunning == true)
		{
			stop();
		}
	}

	void start(ThreadFunction threadFunction, void* userData = NULL, uint32_t stackSize = 0)
	{
		RIO_ASSERT(isRunning == false, "Thread is already running");
		RIO_ASSERT(threadFunction != nullptr, "Function must be != nullptr");
		this->threadFunction = threadFunction;
		this->userData = userData;

#if RIO_PLATFORM_POSIX
		pthread_attr_t attr;
		int err = pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		RIO_ASSERT(err == 0, "pthread_attr_init: errno = %d", err);

		if (stackSize != 0)
		{
			err = pthread_attr_setstacksize(&attr, stackSize);
			RIO_ASSERT(err == 0, "pthread_attr_setstacksize: errno = %d", err);
		}

		err = pthread_create(&threadHandle, &attr, threadProcedure, this);
		RIO_ASSERT(err == 0, "pthread_create: errno = %d", err);

		err = pthread_attr_destroy(&attr);
		RIO_ASSERT(err == 0, "pthread_attr_destroy: errno = %d", err);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		threadHandle = CreateThread(NULL, stackSize, Thread::threadProcedure, this, 0, NULL);
		RIO_ASSERT(threadHandle != NULL, "CreateThread: GetLastError = %d", GetLastError());
#endif // RIO_PLATFORM_WINDOWS
		isRunning = true;
		semaphore.wait();
	}

	void stop()
	{
		RIO_ASSERT(isRunning, "Thread is not running");

#if RIO_PLATFORM_POSIX
		int err = pthread_join(threadHandle, NULL);
		RIO_ASSERT(err == 0, "pthread_join: errno = %d", err);
		RIO_UNUSED(err);
		threadHandle = 0;
#elif RIO_PLATFORM_WINDOWS
		WaitForSingleObject(threadHandle, INFINITE);
		// TODO
		// GetExitCodeThread(threadHandle, &exitCode);
		CloseHandle(threadHandle);
		threadHandle = INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_
		isRunning = false;
	}

	bool getIsRunning()
	{
		return isRunning;
	}

private:

	int32_t run()
	{
		semaphore.post();
		return threadFunction(userData);
	}

#if RIO_PLATFORM_POSIX
	static void* threadProcedure(void* arg)
	{
		static int32_t result = -1;
		result = ((Thread*)arg)->run();
		return (void*)&result;
	}
#elif RIO_PLATFORM_WINDOWS
	static DWORD WINAPI threadProcedure(void* arg)
	{
		Thread* thread = (Thread*)arg;
		int32_t result = thread->run();
		return result;
	}
#endif // RIO_PLATFORM_

private:
	// Disable copying
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;
public:
	ThreadFunction threadFunction = nullptr;
	void* userData = nullptr;
	Semaphore semaphore;
	bool isRunning = false;
#if RIO_PLATFORM_POSIX
	pthread_t threadHandle = 0;
#elif RIO_PLATFORM_WINDOWS
	HANDLE threadHandle = INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka