// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringTypes.h"
#include "Core/Base/Types.h"

namespace Rio
{

struct Guid
{
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint64_t data4;
};

namespace GuidFn
{
	Guid createGuid();
	Guid parse(const char* str);
	bool tryParse(const char* str, Guid& guid);
	void toString(const Guid& guid, DynamicString& s);
} // namespace GuidFn

inline bool operator==(const Guid& a, const Guid& b)
{
	return a.data1 == b.data1
		&& a.data2 == b.data2
		&& a.data3 == b.data3
		&& a.data4 == b.data4
		;
}

const Guid GUID_ZERO = 
{ 
	0u,
	0u,
	0u,
	0u
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka