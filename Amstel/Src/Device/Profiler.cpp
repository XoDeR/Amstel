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
	static char _thread_buffer[THREAD_BUFFER_SIZE];
	static uint32_t _thread_buffer_size = 0;
	static Mutex _buffer_mutex;

	static void flush_local_buffer()
	{
		ScopedMutex sm(_buffer_mutex);
		ArrayFn::push(*ProfilerGlobalFn::buffer, _thread_buffer, _thread_buffer_size);
		_thread_buffer_size = 0;
	}

	template <typename T>
	static void push(ProfilerEventType::Enum type, const T& ev)
	{
		if (_thread_buffer_size + 2 * sizeof(uint32_t) + sizeof(ev) >= THREAD_BUFFER_SIZE)
		{
			flush_local_buffer();
		}

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*)p = type;
		p += sizeof(uint32_t);
		*(uint32_t*)p = sizeof(ev);
		p += sizeof(uint32_t);
		*(T*)p = ev;

		_thread_buffer_size += 2*sizeof(uint32_t) + sizeof(ev);
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
		ProfilerFn::flush_local_buffer();
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