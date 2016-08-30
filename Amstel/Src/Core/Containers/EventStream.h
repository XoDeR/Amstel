// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Containers/Array.h"

namespace Rio
{

struct EventHeader
{
	uint32_t type;
	uint32_t size;
};

// Array of generic event structs
using EventStream = Array<char>;

// The events are stored in the following form:
// [event_header_0][event_data_0][event_header_1][event_data_1] ...

namespace EventStreamFn
{
	// Appends the <event> of the given <type> and <size> to the stream <s>
	inline void write(EventStream& s, uint32_t type, uint32_t size, const void* event)
	{
		EventHeader eventHeader;
		eventHeader.type = type;
		eventHeader.size = size;

		ArrayFn::push(s, (char*)&eventHeader, sizeof(EventHeader));
		ArrayFn::push(s, (char*)event, size);
	}

	// Appends the <event> of the given <type> to the stream <s>
	template <typename T>
	inline void write(EventStream& s, uint32_t type, const T& event)
	{
		EventStreamFn::write(s, type, sizeof(T), &event);
	}
} // namespace EventStreamFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka