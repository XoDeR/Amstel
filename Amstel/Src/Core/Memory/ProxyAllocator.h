// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"

namespace Rio
{

// Offers the facility to tag allocators by a string identifier
// Proxy allocator is appended to a global linked list when instantiated
// so that it is possible to later visit that list for debugging purposes
class ProxyAllocator : public Allocator
{
public:
	// Tag all allocations made with <allocator> by the given <name>
	ProxyAllocator(Allocator& allocator, const char* proxyName);
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);
	void deallocate(void* data);
	uint32_t getAllocatedSize(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }
	uint32_t getTotalAllocatedBytes() { return SIZE_NOT_TRACKED; }
	// Returns the name of the proxy allocator
	const char* getName() const;
private:
	Allocator& allocator;
	const char* proxyName;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka