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
using JsonObject = Map<FixedString, const char*>;

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
