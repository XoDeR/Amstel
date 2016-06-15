// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Error/Error.h"
#include "Device/Profiler.h"

namespace Rio
{

ProxyAllocator::ProxyAllocator(Allocator& allocator, const char* proxyName)
	: allocator(allocator)
	, proxyName(proxyName)
{
	RIO_ASSERT(proxyName != nullptr, "Name must not be NULL");
}

void* ProxyAllocator::allocate(uint32_t size, uint32_t align)
{
	void* p = allocator.allocate(size, align);
	ALLOCATE_MEMORY(proxyName, allocator.getAllocatedSize(p));
	return p;
}

void ProxyAllocator::deallocate(void* data)
{
	DEALLOCATE_MEMORY(proxyName, (data == nullptr) ? 0 : allocator.getAllocatedSize((const void*)data));
	allocator.deallocate(data);
}

const char* ProxyAllocator::getName() const
{
	return proxyName;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka