// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Json/JsonTypes.h"
#include "Core/Strings/StringTypes.h"

namespace Rio
{

// Functions to parse JSON-encoded data
namespace JsonFn
{
	// Returns the data type of the JSON string <json>
	JsonValueType::Enum getType(const char* json);
	// Parses the JSON string <json> and puts it into <string>
	void parseString(const char* json, DynamicString& string);
	// Returns the JSON number <json> as int
	int32_t parseInt(const char* json);
	// Returns the JSON number <json> as double
	double parseDouble(const char* json);
	// Returns the JSON number <json> as float
	float parseFloat(const char* json);
	// Returns the JSON boolean <json> as bool
	bool parseBool(const char* json);
	// Parses the JSON array <json> and puts it into <array> as pointers to
	// the corresponding items into the original <json> string
	void parseArray(const char* json, JsonArray& array);
	// Parses the JSON object <json> and puts it into <object> as map from
	// key to pointer to the corresponding value into the original string <json>
	void parseObject(const char* json, JsonObject& object);
	// Parses the JSON-encoded <json>
	void parse(const char* json, JsonObject& object);
	// Parses the JSON-encoded <json>
	void parse(Buffer& json, JsonObject& object);
} // namespace JsonFn

} // namespace Rio

// Copyright (c) 2016 Volodymyr Syvochka