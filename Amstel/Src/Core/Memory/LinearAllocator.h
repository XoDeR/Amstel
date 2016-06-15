// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"

namespace Rio
{

// Allocates memory linearly from a fixed chunk of memory
// and frees all the allocations with a single call to clear()
class LinearAllocator : public Allocator
{
public:
	// Allocates <size> bytes from <backingAllocator>
	LinearAllocator(Allocator& backingAllocator, uint32_t size);
	// Uses <size> bytes of memory from <start>
	LinearAllocator(void* start, uint32_t size);
	~LinearAllocator();
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);
	// The linear allocator does not support deallocating
	// individual allocations, you have to call clear() to free all allocated memory at once
	void deallocate(void* data);
	// Frees all the allocations made by allocate()
	void clear();
	uint32_t getAllocatedSize(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }
	uint32_t getTotalAllocatedBytes() { return offset; }
private:
	Allocator* backingAllocator = nullptr;
	void* physicalStart = nullptr;
	uint32_t totalSize = 0;
	uint32_t offset = 0;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka