// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Memory/PoolAllocator.h"
#include "Core/Error/Error.h"

namespace Rio
{

PoolAllocator::PoolAllocator(Allocator& backing, uint32_t blockCount, uint32_t blockSize, uint32_t blockAlign)
	: backingAllocator(backing)
	, blockSize(blockSize)
	, blockAlign(blockAlign)
{
	RIO_ASSERT(blockCount > 0, "Unsupported number of blocks");
	RIO_ASSERT(blockSize > 0, "Unsupported block size");
	RIO_ASSERT(blockAlign > 0, "Unsupported block alignment");

	uint32_t actual_block_size = blockSize + blockAlign;
	uint32_t pool_size = blockCount * actual_block_size;

	char* memoryBuffer = (char*) backing.allocate(pool_size, blockAlign);

	// Initialize intrusive freelist
	char* cur = memoryBuffer;
	for (uint32_t bb = 0; bb < blockCount - 1; bb++)
	{
		uintptr_t* next = (uintptr_t*) cur;
		*next = (uintptr_t) cur + actual_block_size;
		cur += actual_block_size;
	}

	uintptr_t* end = (uintptr_t*) cur;
	*end = (uintptr_t) nullptr;

	currentAllocationStart = memoryBuffer;
	freeList = memoryBuffer;
}

PoolAllocator::~PoolAllocator()
{
	backingAllocator.deallocate(currentAllocationStart);
}

void* PoolAllocator::allocate(uint32_t size, uint32_t align)
{
	RIO_ASSERT(size == blockSize, "Size must match block size");
	RIO_ASSERT(align == blockAlign, "Align must match block align");
	RIO_ASSERT(freeList != nullptr, "Out of memory");

	uintptr_t nextFree = *((uintptr_t*)freeList);
	void* userPtr = freeList;
	freeList = (void*)nextFree;

	allocationCount++;
	allocatedSize += blockSize;

	return userPtr;
}

void PoolAllocator::deallocate(void* data)
{
	if (!data)
	{
		return;
	}

	RIO_ASSERT(allocationCount > 0, "Did not allocate");

	uintptr_t* next = (uintptr_t*) data;
	*next = (uintptr_t)freeList;

	freeList = data;

	allocationCount--;
	allocatedSize -= blockSize;
}

uint32_t PoolAllocator::getTotalAllocatedBytes()
{
	return allocatedSize;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka