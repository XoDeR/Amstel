// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringUtils.h"

#include <algorithm> // std::max

namespace Rio
{

struct FixedString
{
	FixedString()
	{
	}

	FixedString(const char* str)
		: length(getStringLength32(str))
		, data(str)
	{
	}

	FixedString(const char* str, uint32_t length)
		: length(length)
		, data(str)
	{
	}

	FixedString& operator=(const char* str)
	{
		this->length = getStringLength32(str);
		this->data = str;
		return *this;
	}

	uint32_t getLength() const
	{
		return length;
	}

	const char* getData() const
	{
		return this->data;
	}

	uint32_t length = 0;
	const char* data = nullptr;
};

inline bool operator==(const FixedString& a, const char* str)
{
	const uint32_t length = getStringLength32(str);
	return a.length == length
		&& strncmp(a.data, str, length) == 0
		;
}

inline bool operator==(const FixedString& a, const FixedString& b)
{
	return a.length == b.length
		&& strncmp(a.data, b.data, a.length) == 0
		;
}

inline bool operator<(const FixedString& a, const FixedString& b)
{
	const uint32_t length = std::max(a.length, b.length);
	return strncmp(a.data, b.data, length) < 0;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka