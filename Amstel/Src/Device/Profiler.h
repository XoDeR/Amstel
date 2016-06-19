// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Math/MathTypes.h"

namespace Rio
{

struct ProfilerEventType
{
	enum Enum
	{
		ENTER_PROFILE_SCOPE,
		LEAVE_PROFILE_SCOPE,
		RECORD_FLOAT,
		RECORD_VECTOR3,
		ALLOCATE_MEMORY,
		DEALLOCATE_MEMORY,

		COUNT
	};
};

struct RecordFloat
{
	const char* name;
	float value;
};

struct RecordVector3
{
	const char* name;
	Vector3 value;
};

struct EnterProfileScope
{
	const char* name;
	int64_t time;
};

struct LeaveProfileScope
{
	int64_t time;
};

struct AllocateMemory
{
	const char* name;
	uint32_t size;
};

struct DeallocateMemory
{
	const char* name;
	uint32_t size;
};

// The profiler does not copy pointer data
// You have to store it somewhere and make sure it is valid throughout the program execution
namespace ProfilerFn
{
	// Starts a new profile scope with the given <name>
	void enterProfileScope(const char* name);

	// Ends the last profile scope
	void leaveProfileScope();

	// Records the float <value> with the given <name>
	void recordFloat(const char* name, float value);

	// Records the vector3 <value> with the given <name>
	void recordVector3(const char* name, const Vector3& value);

	// Records a memory allocation of <size> with the given <name>
	void allocateMemory(const char* name, uint32_t size);

	// Records a memory deallocation of <size> with the given <name>
	void deallocateMemory(const char* name, uint32_t size);
} // namespace ProfilerFn

namespace ProfilerGlobalFn
{
	void init();
	void shutdown();
	const char* getBuffer();
	void flush();
	void clear();
} // namespace ProfilerGlobalFn

} // namespace Rio

#if RIO_DEBUG
	#define ENTER_PROFILE_SCOPE(name) ProfilerFn::enterProfileScope(name)
	#define LEAVE_PROFILE_SCOPE() ProfilerFn::leaveProfileScope()
	#define RECORD_FLOAT(name, value) ProfilerFn::recordFloat(name, value)
	#define RECORD_VECTOR3(name, value) ProfilerFn::recordVector3(name, value)
	#define ALLOCATE_MEMORY(name, size) ProfilerFn::allocateMemory(name, size)
	#define DEALLOCATE_MEMORY(name, size) ProfilerFn::deallocateMemory(name, size)
#else
	#define ENTER_PROFILE_SCOPE(name) ((void)0)
	#define LEAVE_PROFILE_SCOPE() ((void)0)
	#define RECORD_FLOAT(name, value) ((void)0)
	#define RECORD_VECTOR3(name, value) ((void)0)
	#define ALLOCATE_MEMORY(name, size) ((void)0)
	#define DEALLOCATE_MEMORY(name, size) ((void)0)
#endif // RIO_DEBUG

// Copyright (c) 2016 Volodymyr Syvochka