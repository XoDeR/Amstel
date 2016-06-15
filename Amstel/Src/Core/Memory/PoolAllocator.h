// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"

namespace Rio
{

// Allocates fixed-size memory blocks from a fixed memory pool
// The backing allocator is used to allocate the memory pool
class PoolAllocator : public Allocator
{
public:
	// Uses <backing> to allocate the memory pool for containing exactly
	// <blockCount> blocks of <blockSize> size each aligned to <blockAlign>
	PoolAllocator(Allocator& backing, uint32_t blockCount, uint32_t blockSize, uint32_t blockAlign = Allocator::DEFAULT_ALIGN);
	~PoolAllocator();

	// Allocates a block of memory from the memory pool
	// The <size> and <align> must match those passed to PoolAllocator::PoolAllocator()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);
	void deallocate(void* data);
	uint32_t getAllocatedSize(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }
	uint32_t getTotalAllocatedBytes();
private:
	Allocator& backingAllocator;

	void* currentAllocationStart = nullptr;
	void* freeList = nullptr;
	uint32_t blockSize;
	uint32_t blockAlign;

	uint32_t allocationCount = 0;
	uint32_t allocatedSize = 0;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka