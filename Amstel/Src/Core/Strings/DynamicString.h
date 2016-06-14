// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Memory/Memory.h"
#include "Core/Containers/Array.h"
#include "Core/Strings/StringId.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Strings/FixedString.h"

#include <string.h> // memmove, strncmp

namespace Rio
{

// Dynamic array of characters.
struct DynamicString
{
	ALLOCATOR_AWARE;

	DynamicString(Allocator& a);
	DynamicString(const char* str, Allocator& a = getDefaultAllocator());

	DynamicString& operator=(const DynamicString& ds);
	DynamicString& operator=(const char* str);
	DynamicString& operator=(const char c);
	DynamicString& operator=(const FixedString& fs);

	// Sets the string to another <str>
	void set(const char* str, uint32_t length);
	// Reserves space for at least <n> characters
	void reserve(uint32_t n);
	uint32_t getLength() const;
	// Returns whether the string is empty
	bool getIsEmpty() const;
	// Removes leading white-space characters from the string
	void trimLeading();
	// Removes trailing white-space characters from the string
	void trimTrailing();
	// Removes leading and trailing white-space characters from the string
	void trim();
	// Returns whether the string starts with <str>
	bool hasPrefix(const char* str) const;
	// Returns whether the string ends with <str>
	bool hasSuffix(const char* str) const;
	// Returns the StringId32 of the string
	StringId32 getStringId() const;
	// Returns the string as a NULL-terminated string
	const char* getCStr() const;

	Array<char> data;
};

inline DynamicString::DynamicString(Allocator& a)
	: data(a)
{
}

inline DynamicString::DynamicString(const char* str, Allocator& a)
	: data(a)
{
	RIO_ASSERT_NOT_NULL(str);
	ArrayFn::push(data, str, getStringLength32(str));
}

inline void DynamicString::set(const char* str, uint32_t length)
{
	ArrayFn::resize(data, length);
	strncpy(ArrayFn::begin(data), str, length);
}

// Appends the string <b> to <a>
inline DynamicString& operator+=(DynamicString& a, const DynamicString& b)
{
	ArrayFn::push(a.data, ArrayFn::begin(b.data), ArrayFn::getCount(b.data));
	return a;
}

// Appends the string <str> to <a>
inline DynamicString& operator+=(DynamicString& a, const char* str)
{
	RIO_ASSERT_NOT_NULL(str);
	ArrayFn::push(a.data, str, getStringLength32(str));
	return a;
}

// Appends the char <c> to <a>
inline DynamicString& operator+=(DynamicString& a, const char c)
{
	ArrayFn::pushBack(a.data, c);
	return a;
}

// Appends the string <fs> to <a>
inline DynamicString& operator+=(DynamicString& a, const FixedString& fs)
{
	ArrayFn::push(a.data, fs.getData(), fs.getLength());
	return a;
}

inline DynamicString& DynamicString::operator=(const DynamicString& ds)
{
	this->data = ds.data;
	return *this;
}

inline DynamicString& DynamicString::operator=(const char* str)
{
	RIO_ASSERT_NOT_NULL(str);
	ArrayFn::clear(this->data);
	ArrayFn::push(this->data, str, getStringLength32(str));
	return *this;
}

inline DynamicString& DynamicString::operator=(const char c)
{
	ArrayFn::clear(this->data);
	ArrayFn::pushBack(this->data, c);
	return *this;
}

inline DynamicString& DynamicString::operator=(const FixedString& fs)
{
	ArrayFn::clear(this->data);
	ArrayFn::push(this->data, fs.getData(), fs.getLength());
	return *this;
}

inline bool operator<(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.getCStr(), b.getCStr()) < 0;
}

inline bool operator==(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.getCStr(), b.getCStr()) == 0;
}

inline bool operator==(const DynamicString& a, const char* str)
{
	RIO_ASSERT_NOT_NULL(str);
	return strcmp(a.getCStr(), str) == 0;
}

inline void DynamicString::reserve(uint32_t n)
{
	ArrayFn::reserve(this->data, n);
}

inline uint32_t DynamicString::getLength() const
{
	return getStringLength32(getCStr());
}

inline bool DynamicString::getIsEmpty() const
{
	return getLength() == 0;
}

inline void DynamicString::trimLeading()
{
	const char* str = getCStr();
	const char* end = skipSpaces(str);

	const uint32_t length = getStringLength32(end);

	memmove(ArrayFn::begin(this->data), end, length);
	ArrayFn::resize(this->data, length);
}

inline void DynamicString::trimTrailing()
{
	char* str = (char*)getCStr();
	char* end = str + getStringLength32(str) - 1;

	while (end > str && isspace(*end)) --end;

	*(end + 1) = '\0';
}

inline void DynamicString::trim()
{
	trimLeading();
	trimTrailing();
}

inline bool DynamicString::hasPrefix(const char* str) const
{
	RIO_ASSERT_NOT_NULL(str);
	const uint32_t ml = getStringLength32(getCStr());
	const uint32_t sl = getStringLength32(str);
	return sl <= ml && strncmp(&this->data[0], str, sl) == 0;
}

inline bool DynamicString::hasSuffix(const char* str) const
{
	RIO_ASSERT_NOT_NULL(str);
	const uint32_t ml = getStringLength32(getCStr());
	const uint32_t sl = getStringLength32(str);
	return sl <= ml && strncmp(&this->data[ml-sl], str, sl) == 0;
}

inline StringId32 DynamicString::getStringId() const
{
	return StringId32(getCStr());
}

inline const char* DynamicString::getCStr() const
{
	Array<char>& data = const_cast<Array<char>& >(this->data);
	ArrayFn::pushBack(data, '\0');
	ArrayFn::popBack(data);
	return ArrayFn::begin(this->data);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka