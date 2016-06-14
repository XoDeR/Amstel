// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringTypes.h"
#include "Core/Base/Types.h"

namespace Rio
{

// Hashed string
struct StringId32
{
	StringId32() 
	{}
	explicit StringId32(uint32_t idx) 
		: id(idx) 
	{
	}
	explicit StringId32(const char* str);
	explicit StringId32(const char* str, uint32_t length);

	void hash(const char* str, uint32_t length);

	// Fills s with the string representation of the id
	void toString(DynamicString& s);

	uint32_t id = 0;
};

// Hashed string
struct StringId64
{
	StringId64() 
	{}
	explicit StringId64(uint64_t idx) 
		: id(idx) 
	{}
	explicit StringId64(const char* str);
	explicit StringId64(const char* str, uint32_t length);

	void hash(const char* str, uint32_t length);
	// Fills s with the string representation of the id
	void toString(DynamicString& s);

	uint64_t id = 0;
};

inline bool operator==(const StringId32& a, const StringId32& b)
{
	return a.id == b.id;
}

inline bool operator!=(const StringId32& a, const StringId32& b)
{
	return a.id != b.id;
}

inline bool operator<(const StringId32& a, const StringId32& b)
{
	return a.id < b.id;
}

inline bool operator==(const StringId64& a, const StringId64& b)
{
	return a.id == b.id;
}

inline bool operator!=(const StringId64& a, const StringId64& b)
{
	return a.id != b.id;
}

inline bool operator<(const StringId64& a, const StringId64& b)
{
	return a.id < b.id;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka