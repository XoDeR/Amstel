// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"
#include "Core/Memory/Memory.h"

namespace Rio
{
	// A temporary memory allocator that primarily allocates memory from a local stack buffer of size BUFFER_SIZE
	// If that memory is exhausted it will use the backing allocator (typically a scratch allocator)
	// Memory allocated with a TempAllocator does not have to be deallocated
	// It is automatically deallocated when the TempAllocator is destroyed
	template <int BUFFER_SIZE>
	class TempAllocator : public Allocator
	{
	public:
		TempAllocator(Allocator &backing = getDefaultScratchAllocator());
		virtual ~TempAllocator();
		virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN);

		// Deallocation is a NOP for the TempAllocator. The memory is automatically
		// deallocated when the TempAllocator is destroyed.
		virtual void deallocate(void *) {}
		// Returns SIZE_NOT_TRACKED
		virtual uint32_t getAllocatedSize(const void*) { return SIZE_NOT_TRACKED; }
		// Returns SIZE_NOT_TRACKED
		virtual uint32_t getTotalAllocatedBytes() { return SIZE_NOT_TRACKED; }
	private:
		char buffer[BUFFER_SIZE]; // Local stack buffer for allocations
		Allocator& backingAllocator; // Backing allocator if local memory is exhausted
		char* currentAllocationStart; // Start of current allocation region
		char* currentAllocationPointer;	// Current allocation pointer
		char* currentAllocationEnd;	// End of current allocation region
		unsigned chunkSize;	// Chunks to allocate from backing allocator
	};

	// If possible, use one of these predefined sizes for the TempAllocator to avoid unnecessary template instantiation.
	typedef TempAllocator<64> TempAllocator64;
	typedef TempAllocator<128> TempAllocator128;
	typedef TempAllocator<256> TempAllocator256;
	typedef TempAllocator<512> TempAllocator512;
	typedef TempAllocator<1024> TempAllocator1024;
	typedef TempAllocator<2048> TempAllocator2048;
	typedef TempAllocator<4096> TempAllocator4096;

	///////////////////////////////////
	// Inline function implementations
	///////////////////////////////////

	template <int BUFFER_SIZE>
	TempAllocator<BUFFER_SIZE>::TempAllocator(Allocator& backingAllocator) 
		: backingAllocator(backingAllocator)
		, chunkSize(4*1024)
	{
		currentAllocationPointer = currentAllocationStart = buffer;
		currentAllocationEnd = currentAllocationStart + BUFFER_SIZE;
		*(void **)currentAllocationStart = 0;
		currentAllocationPointer += sizeof(void *);
	}

	template <int BUFFER_SIZE>
	TempAllocator<BUFFER_SIZE>::~TempAllocator()
	{
		char* start = buffer;
		void *p = *(void **)start;
		while (p) 
		{
			void* next = *(void **)p;
			backingAllocator.deallocate(p);
			p = next;
		}
	}

	template <int BUFFER_SIZE>
	void *TempAllocator<BUFFER_SIZE>::allocate(uint32_t size, uint32_t align)
	{
		currentAllocationPointer = (char *)MemoryFn::alignTop(currentAllocationPointer, align);
		if ((int)size > currentAllocationEnd - currentAllocationPointer)
		{
			uint32_t toAllocate = sizeof(void *) + size + align;
			if (toAllocate < chunkSize)
			{
				toAllocate = chunkSize;
			}
			chunkSize *= 2;
			void *p = backingAllocator.allocate(toAllocate);
			*(void **)currentAllocationStart = p;
			currentAllocationPointer = currentAllocationStart = (char *)p;
			currentAllocationEnd = currentAllocationStart + toAllocate;
			*(void **)currentAllocationStart = 0;
			currentAllocationPointer += sizeof(void *);
			currentAllocationPointer = (char *)MemoryFn::alignTop(currentAllocationPointer, align);
		}
		void *result = currentAllocationPointer;
		currentAllocationPointer += size;
		return result;
	}
} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka