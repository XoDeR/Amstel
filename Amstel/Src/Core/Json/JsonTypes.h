// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Strings/FixedString.h"

namespace Rio
{

struct JsonValueType
{
	enum Enum
	{
		NIL,
		BOOL,
		NUMBER,
		STRING,
		ARRAY,
		OBJECT
	};
};

// Array of pointers to json-encoded data
using JsonArray = Array<const char*>;

// Map from key to pointers to json-encoded data
struct JsonObject
{
	JsonObject(Allocator& a);

	const char* operator[](const char* key) const;
	const char* operator[](const FixedString& key) const;

	Map<FixedString, const char*> map;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
