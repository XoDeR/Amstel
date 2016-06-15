// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Memory/StackAllocator.h"
#include "Core/Memory/Memory.h"

namespace Rio
{

StackAllocator::StackAllocator(char* begin, uint32_t size)
	: stackBegin(begin)
	, stackTop(begin)
	, totalSize(size)
{
}

StackAllocator::~StackAllocator()
{
	RIO_ASSERT(allocationCount == 0 && getTotalAllocatedBytes() == 0
		, "Missing %d deallocations causing a leak of %d bytes"
		, allocationCount
		, getTotalAllocatedBytes()
		);
}

void* StackAllocator::allocate(uint32_t size, uint32_t align)
{
	void* result = nullptr;

	const uint32_t actualSize = sizeof(Header) + size + align;

	// Memory exhausted
	if (stackTop + actualSize > stackBegin + totalSize)
	{
		return nullptr;
	}

	// The offset from TOS to the start of the buffer
	uint32_t offset = uint32_t(stackTop - stackBegin);

	// Align user data only, ignore header alignment
	stackTop = (char*)MemoryFn::alignTop(stackTop + sizeof(Header), align) - sizeof(Header);

	Header* header = (Header*)stackTop;
	header->offset = offset;
	header->allocationId = allocationCount;

	void* userPtr = stackTop + sizeof(Header);
	stackTop = stackTop + actualSize;

	allocationCount++;

	return userPtr;
}

void StackAllocator::deallocate(void* data)
{
	if (!data)
	{
		return;
	}

	Header* dataHeader = (Header*)((char*)data - sizeof(Header));

	RIO_ASSERT(dataHeader->allocationId == allocationCount - 1, "Deallocations must occur in LIFO order");

	stackTop = stackBegin + dataHeader->offset;

	allocationCount--;
}

uint32_t StackAllocator::getTotalAllocatedBytes()
{
	return uint32_t(stackTop - stackBegin);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka