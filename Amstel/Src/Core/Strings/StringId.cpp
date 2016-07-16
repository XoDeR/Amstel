// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Strings/StringId.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Error/Error.h"
#include "Core/Base/Murmur.h"
#include "Core/Compat/Msvc/IntTypes.h" // PRIx64

namespace Rio
{

StringId32::StringId32(const char* str)
{
	this->hash(str, getStringLength32(str));
}

StringId32::StringId32(const char* str, uint32_t length)
{
	this->hash(str, length);
}

void StringId32::hash(const char* str, uint32_t length)
{
	RIO_ASSERT_NOT_NULL(str);
	id = getMurmurHash32(str, length, 0);
}

void StringId32::toString(DynamicString& s)
{
	char buffer[8+1];
	snPrintF(buffer, sizeof(buffer), "%.8x", id);
	s.set(buffer, sizeof(buffer)-1);
}

StringId64::StringId64(const char* str)
{
	this->hash(str, getStringLength32(str));
}

StringId64::StringId64(const char* str, uint32_t length)
{
	this->hash(str, length);
}

void StringId64::hash(const char* str, uint32_t length)
{
	RIO_ASSERT_NOT_NULL(str);
	id = getMurmurHash64(str, length, 0);
}

void StringId64::toString(DynamicString& s)
{
	char buffer[16+1];
	snPrintF(buffer, sizeof(buffer), "%.16" PRIx64, id);
	s.set(buffer, sizeof(buffer) - 1);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka