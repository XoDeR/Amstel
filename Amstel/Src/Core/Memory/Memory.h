// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Error/Error.h"
#include "Core/Memory/Allocator.h"

#include <new>

namespace Rio
{

Allocator& getDefaultAllocator();
Allocator& getDefaultScratchAllocator();

namespace MemoryFn
{
	// Returns the pointer aligned to the desired align byte
	inline void* alignTop(void* p, uint32_t align)
	{
		RIO_ASSERT(align >= 1, "Alignment must be > 1");
		RIO_ASSERT(align % 2 == 0 || align == 1, "Alignment must be a power of two");

		uintptr_t ptr = (uintptr_t)p;
		const uint32_t mod = ptr % align;

		if (mod)
		{
			ptr += align - mod;
		}

		return (void*)ptr;
	}

	// Respects standard behavior when calling on nullptr
	template <typename T>
	inline void callDestructorAndDeallocate(Allocator& a, T* ptr)
	{
		if (ptr == nullptr)
		{
			return;
		}

		ptr->~T();
		a.deallocate(ptr);
	}
} // namespace MemoryFn

#define ALLOCATOR_AWARE using AllocatorAware = int

// Convert integer to type.
template <int v>
struct TIntToType 
{ 
	enum 
	{
		value = v
	}; 
};

// Determines if a class is allocator aware.
template <class T>
struct TIsAllocatorAware 
{
	template <typename C>
	static char testFunction(typename C::AllocatorAware *);

	template <typename C>
	static int testFunction(...);
public:
	enum 
	{
		value = (sizeof(testFunction<T>(0)) == sizeof(char))
	};
};

#define IS_ALLOCATOR_AWARE(T) TIsAllocatorAware<T>::value
#define IS_ALLOCATOR_AWARE_TYPE(T) TIntToType<IS_ALLOCATOR_AWARE(T)>

// Allocator aware constuction
template <class T> inline T& construct(void *p, Allocator& a, TIntToType<true>)
{
	new (p) T(a); return *(T *)p;
}

template <class T> inline T& construct(void *p, Allocator& /*a*/, TIntToType<false>)
{
	new (p) T; return *(T *)p;
}

template <class T> inline T& construct(void *p, Allocator& a) 
{
	return construct<T>(p, a, IS_ALLOCATOR_AWARE_TYPE(T)());
}

namespace MemoryGlobalFn
{
	// Constructs the initial default allocators
	// Has to be called before anything else during the engine startup
	void init();
	// Destroys the allocators created with MemoryGlobalFn::init()
	// Should be the last call of the program
	void shutdown();
} // namespace MemoryGlobalFn

} // namespace Rio

// Allocates memory with <allocator> for the given <T> type and calls constructor on it
// <allocator> must be a reference to an existing allocator
#define RIO_NEW(allocator, T) new ((allocator).allocate(sizeof(T), alignof(T))) T

// Calls destructor on the <ptr> and deallocates memory using the <allocator>
// The allocator must be a reference to an existing allocator
#define RIO_DELETE(allocator, ptr) Rio::MemoryFn::callDestructorAndDeallocate(allocator, ptr)

// Copyright (c) 2016 Volodymyr Syvochka