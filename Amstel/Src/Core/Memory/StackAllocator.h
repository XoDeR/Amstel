// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"

namespace Rio
{

// Allocates memory linearly in a stack-like fashion from a predefined chunk
// All deallocations must occur in LIFO order
class StackAllocator : public Allocator
{
private:
	struct Header
	{
		uint32_t offset;
		uint32_t allocationId;
	};
public:
	StackAllocator(char* begin, uint32_t size);
	~StackAllocator();
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	// Deallocations must occur in LIFO order;
	// the last allocation must be freed for first
	void deallocate(void* data);
	uint32_t getAllocatedSize(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }
	uint32_t getTotalAllocatedBytes();
private:
	char* stackBegin;
	char* stackTop;
	uint32_t totalSize;
	uint32_t allocationCount = 0;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka