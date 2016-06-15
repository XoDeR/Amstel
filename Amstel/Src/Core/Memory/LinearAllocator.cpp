// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/Memory.h"

namespace Rio
{

LinearAllocator::LinearAllocator(Allocator& backingAllocator, uint32_t size)
	: backingAllocator(&backingAllocator)
	, totalSize(size)
{
	physicalStart = this->backingAllocator->allocate(size);
}

LinearAllocator::LinearAllocator(void* start, uint32_t size)
	: physicalStart(start)
	, totalSize(size)
{
}

LinearAllocator::~LinearAllocator()
{
	if (backingAllocator != nullptr)
	{
		backingAllocator->deallocate(physicalStart);
	}

	RIO_ASSERT(offset == 0
		, "Memory leak of %d bytes, maybe you forgot to call clear()?"
		, offset
		);
}

void* LinearAllocator::allocate(uint32_t size, uint32_t align)
{
	void* result = nullptr;

	const uint32_t actualSize = size + align;

	if (offset + actualSize > totalSize)
	{
		// Out of memory
	}
	else
	{
		result = MemoryFn::alignTop(static_cast<char*>(physicalStart) + offset, align);
		offset += actualSize;
	}
	return result;
}

void LinearAllocator::deallocate(void* /*data*/)
{
	// Single deallocations not supported. Use clear().
}

void LinearAllocator::clear()
{
	offset = 0;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka