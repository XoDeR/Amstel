// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Containers/ContainerTypes.h"

namespace Rio
{

struct DisplayMode
{
	uint32_t id;
	uint32_t width;
	uint32_t height;
};

struct Display
{
	virtual void getModes(Array<DisplayMode>& modes) = 0;
	// The initial display mode is automatically reset when the program terminates
	virtual void setMode(uint32_t id) = 0;
};

namespace DisplayFn
{
	Display* create(Allocator& a);
	void destroy(Allocator& a, Display& d);
} // namespace DisplayFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka