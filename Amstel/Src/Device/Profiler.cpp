// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/Profiler.h"

#include "Core/Base/Os.h"
#include "Core/Containers/Array.h"
#include "Core/Thread/Mutex.h"
#include "Core/Memory/Memory.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace ProfilerGlobalFn
{
	char memoryBuffer[sizeof(Buffer)];
	Buffer* buffer = nullptr;

	void init()
	{
		buffer = new (memoryBuffer)Buffer(getDefaultAllocator());
	}

	void shutdown()
	{
		buffer->~Buffer();
		buffer = nullptr;
	}

	const char* getBuffer()
	{
		return ArrayFn::begin(*buffer);
	}
} // namespace ProfilerGlobalFn

namespace ProfilerFn
{
	enum { THREAD_BUFFER_SIZE = 4 * 1024 };
	static char threadBuffer[THREAD_BUFFER_SIZE];
	static uint32_t threadBufferSize = 0;
	static Mutex bufferMutex;

	static void flushLocalBuffer()
	{
		ScopedMutex scopedMutex(bufferMutex);
		ArrayFn::push(*ProfilerGlobalFn::buffer, threadBuffer, threadBufferSize);
		threadBufferSize = 0;
	}

	template <typename T>
	static void push(ProfilerEventType::Enum type, const T& ev)
	{
		if (threadBufferSize + 2 * sizeof(uint32_t) + sizeof(ev) >= THREAD_BUFFER_SIZE)
		{
			flushLocalBuffer();
		}

		char* p = threadBuffer + threadBufferSize;
		*(uint32_t*)p = type;
		p += sizeof(uint32_t);
		*(uint32_t*)p = sizeof(ev);
		p += sizeof(uint32_t);
		*(T*)p = ev;

		threadBufferSize += 2 * sizeof(uint32_t) + sizeof(ev);
	}

	void enterProfileScope(const char* name)
	{
		EnterProfileScope ev;
		ev.name = name;
		ev.time = OsFn::getClockTime();

		push(ProfilerEventType::ENTER_PROFILE_SCOPE, ev);
	}

	void leaveProfileScope()
	{
		LeaveProfileScope ev;
		ev.time = OsFn::getClockTime();

		push(ProfilerEventType::LEAVE_PROFILE_SCOPE, ev);
	}

	void recordFloat(const char* name, float value)
	{
		RecordFloat ev;
		ev.name = name;
		ev.value = value;

		push(ProfilerEventType::RECORD_FLOAT, ev);
	}

	void recordVector3(const char* name, const Vector3& value)
	{
		RecordVector3 ev;
		ev.name = name;
		ev.value = value;

		push(ProfilerEventType::RECORD_VECTOR3, ev);
	}

	void allocateMemory(const char* name, uint32_t size)
	{
		AllocateMemory ev;
		ev.name = name;
		ev.size = size;

		push(ProfilerEventType::ALLOCATE_MEMORY, ev);
	}

	void deallocateMemory(const char* name, uint32_t size)
	{
		DeallocateMemory ev;
		ev.name = name;
		ev.size = size;

		push(ProfilerEventType::DEALLOCATE_MEMORY, ev);
	}
} // namespace ProfilerFn

namespace ProfilerGlobalFn
{
	void flush()
	{
		ProfilerFn::flushLocalBuffer();
		uint32_t end = ProfilerEventType::COUNT;
		ArrayFn::push(*buffer, (const char*)&end, (uint32_t)sizeof(end));
	}

	void clear()
	{
		ArrayFn::clear(*buffer);
	}
}

} // namespace Rio

// Copyright (c) 2016 Volodymyr Syvochka