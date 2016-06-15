// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

class Allocator
{
public:
	Allocator() {}
	virtual	~Allocator() {}

	virtual void* allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;
	virtual void deallocate(void* data) = 0;
	virtual uint32_t getAllocatedSize(const void* ptr) = 0;
	virtual uint32_t getTotalAllocatedBytes() = 0;

	// Default memory alignment in bytes
	static const uint32_t DEFAULT_ALIGN = 4;
	static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;
private:
	// Disable copying
	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka