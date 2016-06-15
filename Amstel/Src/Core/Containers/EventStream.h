// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Containers/Array.h"

namespace Rio
{

// Array of generic event structs
using EventStream = Array<char>;

// The events are stored in the following form:
// [event_header_0][event_data_0][event_header_1][event_data_1] ...

namespace EventStreamFn
{
	struct Header
	{
		uint32_t type;
		uint32_t size;
	};

	// Appends the <event> of the given <type> and <size> to the stream <s>
	inline void write(EventStream& s, uint32_t type, uint32_t size, const void* event)
	{
		Header header;
		header.type = type;
		header.size = size;

		ArrayFn::push(s, (char*)&header, sizeof(Header));
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