// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/Array.h"
#include "Core/Strings/StringTypes.h"
#include "Core/Strings/StringUtils.h"

namespace Rio
{

namespace StringStreamFn
{
	// Returns the stream as a C NULL-terminated string.
	const char* getCStr(StringStream& s);
	template <typename T> StringStream& streamPrintF(StringStream& s, const char* format, T& val);

} // namespace StringStreamFn

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, char val)
{
	ArrayFn::pushBack(s, val);
	return s;
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, int16_t val)
{
	return StringStreamFn::streamPrintF(s, "%hd", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, uint16_t val)
{
	return StringStreamFn::streamPrintF(s, "%hu", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, int32_t val)
{
	return StringStreamFn::streamPrintF(s, "%d", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, uint32_t val)
{
	return StringStreamFn::streamPrintF(s, "%u", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, int64_t val)
{
	return StringStreamFn::streamPrintF(s, "%lld", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, uint64_t val)
{
	return StringStreamFn::streamPrintF(s, "%llu", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, float val)
{
	return StringStreamFn::streamPrintF(s, "%g", val);
}

// Appends <val> to the stream <s> using appropriate formatting
inline StringStream& operator<<(StringStream& s, double val)
{
	return StringStreamFn::streamPrintF(s, "%g", val);
}

// Appends the string to the stream
inline StringStream& operator<<(StringStream& s, const char* str)
{
	ArrayFn::push(s, str, getStringLength32(str));
	return s;
}

namespace StringStreamFn
{
	inline const char* getCStr(StringStream& s)
	{
		ArrayFn::pushBack(s, '\0');
		ArrayFn::popBack(s);
		return ArrayFn::begin(s);
	}

	template <typename T>
	inline StringStream& streamPrintF(StringStream& s, const char* format, T& val)
	{
		char buffer[32];
		snPrintF(buffer, sizeof(buffer), format, val);
		return s << buffer;
	}
} // namespace StringStreamFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka