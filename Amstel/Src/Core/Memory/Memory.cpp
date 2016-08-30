// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Memory/Memory.h"
#include "Core/Memory/Allocator.h"
#include "Core/Thread/Mutex.h"

#include <stdlib.h> // malloc

#define CPP_NEW_DELETE_DISABLED 0
#if CPP_NEW_DELETE_DISABLED

// usual global operators delete and new forbidden
 void* operator new(size_t) throw (std::bad_alloc)
 {
 	RIO_FATAL("operator new forbidden");
 	return nullptr;
 }

 void* operator new[](size_t) throw (std::bad_alloc)
 {
	RIO_FATAL("operator new[] forbidden");
	return nullptr;
 }

 void operator delete(void*) throw ()
 {
	 RIO_FATAL("operator delete forbidden");
 }

 void operator delete[](void*) throw ()
 {
	 RIO_FATAL("operator delete[] forbidden");
 }
#endif //CPP_NEW_DELETE_DISABLED

namespace Rio
{

namespace MemoryFn
{
	// Header stored at the beginning of a memory allocation to indicate the
	// size of the allocated data
	struct Header
	{
		uint32_t size;
	};

	// If we need to align the memory allocation we pad the header with this
	// value after storing the size
	const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

	// Given a pointer to the header, returns a pointer to the data that follows it
	inline void* getDataPointer(Header* header, uint32_t align) 
	{
		void* p = header + 1;
		return MemoryFn::alignTop(p, align);
	}

	// Given a pointer to the data, returns a pointer to the header before it
	inline Header* header(const void* data)
	{
		uint32_t *p = (uint32_t*)data;
		while (p[-1] == HEADER_PAD_VALUE)
		{
			--p;
		}
		return (Header*)p - 1;
	}

	// Stores the size in the header and pads with HEADER_PAD_VALUE up to the data pointer
	inline void fill(Header*header, void *data, uint32_t size)
	{
		header->size = size;
		uint32_t *p = (uint32_t*)(header + 1);
		while (p < data)
		{
			*p++ = HEADER_PAD_VALUE;
		}
	}

	inline uint32_t getActualAllocationSize(uint32_t size, uint32_t align)
	{
		return size + align + sizeof(Header);
	}

	inline void pad(Header* header, void* data)
	{
		uint32_t* p = (uint32_t*)(header + 1);

		while (p != data)
		{
			*p = HEADER_PAD_VALUE;
			p++;
		}
	}

	// Allocator based on C malloc()
	class HeapAllocator : public Allocator
	{
	public:
		HeapAllocator()
		{
		}

		~HeapAllocator()
		{
			RIO_ASSERT(allocationCount == 0 && getTotalAllocatedBytes() == 0
				, "Missing %d deallocations causing a leak of %d bytes"
				, allocationCount
				, getTotalAllocatedBytes()
				);
		}

		void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN)
		{
			ScopedMutex scopedMutex(mutex);

			uint32_t actualSize = getActualAllocationSize(size, align);

			Header* h = (Header*)malloc(actualSize);
			h->size = actualSize;

			void* data = MemoryFn::alignTop(h + 1, align);

			pad(h, data);

			allocatedSize += actualSize;
			allocationCount++;

			return data;
		}

		void deallocate(void* data)
		{
			ScopedMutex scopedMutex(mutex);

			if (!data)
			{
				return;
			}

			Header* h = header(data);

			allocatedSize -= h->size;
			allocationCount--;

			free(h);
		}

		uint32_t getAllocatedSize(const void* ptr)
		{
			return getSize(ptr);
		}

		uint32_t getTotalAllocatedBytes()
		{
			ScopedMutex scopedMutex(mutex);
			return allocatedSize;
		}

		// Returns the size (in bytes) of the block of memory pointed by <data>
		uint32_t getSize(const void* data)
		{
			ScopedMutex scopedMutex(mutex);
			Header* h = header(data);
			return h->size;
		}
	private:
		Mutex mutex;
		uint32_t allocatedSize = 0;
		uint32_t allocationCount = 0;
	};

	// An allocator used to allocate temporary "scratch" memory
	// Uses a fixed size ring buffer to service the requests
	// Memory is always always allocated linearly
	// An allocation pointer is advanced through the buffer as memory is allocated and wraps around at
	// the end of the buffer. 
	// Similarly, a free pointer is advanced as memory is freed
	// It is important that the scratch allocator is only used for short-lived memory allocations
	// A long lived allocator will lock the "free" pointer
	// and prevent the "allocate" pointer from proceeding past it, which means the ring buffer can't be used 
	// If the ring buffer is exhausted, the scratch allocator will use its backing allocator to allocate memory instead
	class ScratchAllocator : public Allocator
	{
	public:
		// ScratchAllocator will use the <backingAllocator> to create the ring buffer 
		// and to service any requests that don't fit in the ring buffer
		// <size> specifies the size of the ring buffer.
		ScratchAllocator(Allocator& backingAllocator, uint32_t size) 
			: backingAllocator(backingAllocator)
		{
			ringBufferBegin = (char*)this->backingAllocator.allocate(size);
			ringBufferEnd = ringBufferBegin + size;
			whereToAllocate = ringBufferBegin;
			whereToFree = ringBufferBegin;
		}

		~ScratchAllocator()
		{
			RIO_ASSERT(whereToFree == whereToAllocate, "Memory leak");
			backingAllocator.deallocate(ringBufferBegin);
		}

		bool getIsInUse(void *p)
		{
			if (whereToFree == whereToAllocate)
			{
				return false;
			}
			if (whereToAllocate > whereToFree)
			{
				return p >= whereToFree && p < whereToAllocate;
			}
			return p >= whereToFree || p < whereToAllocate;
		}

		void* allocate(uint32_t size, uint32_t align)
		{
			ScopedMutex scopedMutex(mutex);

			RIO_ASSERT(align % 4 == 0, "Must be 4-byte aligned");
			size = ((size + 3)/4) * 4;

			char* p = whereToAllocate;
			Header* h = (Header*)p;
			char* data = (char*)getDataPointer(h, align);
			p = data + size;

			// Reached the end of the buffer, wrap around to the beginning.
			if (p > ringBufferEnd)
			{
				h->size = uint32_t(ringBufferEnd - (char*)h) | 0x80000000u;

				p = ringBufferBegin;
				h = (Header*)p;
				data = (char*)getDataPointer(h, align);
				p = data + size;
			}

			// If the buffer is exhausted use the backing allocator instead
			if (getIsInUse(p))
			{
				return backingAllocator.allocate(size, align);
			}

			fill(h, data, uint32_t(p - (char*)h));
			whereToAllocate = p;
			return data;
		}

		void deallocate(void *p)
		{
			ScopedMutex scopedMutex(mutex);

			if (!p)
			{
				return;
			}

			if (p < ringBufferBegin || p >= ringBufferEnd)
			{
				backingAllocator.deallocate(p);
				return;
			}

			// Mark this slot as free
			Header*h = header(p);
			RIO_ASSERT((h->size & 0x80000000u) == 0, "Not free");
			h->size = h->size | 0x80000000u;

			// Advance the free pointer past all free slots
			while (whereToFree != whereToAllocate)
			{
				Header*h = (Header*)whereToFree;
				if ((h->size & 0x80000000u) == 0)
				{
					break;
				}

				whereToFree += h->size & 0x7fffffffu;
				if (whereToFree == ringBufferEnd)
				{
					whereToFree = ringBufferBegin;
				}
			}
		}

		uint32_t getAllocatedSize(const void *p)
		{
			ScopedMutex scopedMutex(mutex);
			Header* h = header(p);
			return h->size - uint32_t((char*)p - (char*)h);
		}

		uint32_t getTotalAllocatedBytes()
		{
			ScopedMutex scopedMutex(mutex);
			return uint32_t(ringBufferEnd - ringBufferBegin);
		}

	private:
		Mutex mutex;
		Allocator& backingAllocator;

		// Start and end of the ring buffer
		char* ringBufferBegin;
		char* ringBufferEnd;

		// Pointers to where to allocate memory and where to free memory
		char* whereToAllocate;
		char* whereToFree;
	};
} // namespace MemoryFn

namespace MemoryGlobalFn
{
	using namespace MemoryFn;

	static const uint32_t SIZE = sizeof(HeapAllocator) + sizeof(ScratchAllocator);
	char buffer[SIZE];
	HeapAllocator* defaultAllocator = nullptr;
	ScratchAllocator* defaultScratchAllocator = nullptr;

	void init()
	{
		defaultAllocator = new (buffer) HeapAllocator();
		defaultScratchAllocator = new (buffer + sizeof(HeapAllocator)) ScratchAllocator(*defaultAllocator, 1024*1024);
	}

	void shutdown()
	{
		defaultScratchAllocator->~ScratchAllocator();
		defaultAllocator->~HeapAllocator();
	}
} // namespace MemoryGlobalFn

Allocator& getDefaultAllocator()
{
	return *MemoryGlobalFn::defaultAllocator;
}

Allocator& getDefaultScratchAllocator()
{
	return *MemoryGlobalFn::defaultScratchAllocator;
}

} // namespace Rio

// Copyright (c) 2016 Volodymyr Syvochka