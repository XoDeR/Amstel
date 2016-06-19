// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Json/JsonTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringTypes.h"

namespace Rio
{

namespace JsonRFn
{
	// Returns the data type of the JSONR string <json>
	JsonValueType::Enum getType(const char* json);

	// Parses the JSONR string <json> and puts it into <string>
	void parseString(const char* json, DynamicString& string);

	// Returns the JSONR number <json> as double
	double parseDouble(const char* json);

	// Returns the JSONR number <json> as int
	int32_t parseInt(const char* json);

	// Returns the JSONR number <json> as float
	float parseFloat(const char* json);

	// Returns the JSONR boolean <json> as bool
	bool parseBool(const char* json);

	// Parses the JSONR array <json> and puts it into <array> as pointers to
	// the corresponding items into the original <json> string
	void parseArray(const char* json, JsonArray& array);

	// Parses the JSONR object <json> and puts it into <object> as map from
	// key to pointer to the corresponding value into the original string <json>
	void parseObject(const char* json, JsonObject& object);

	// Parses the JSONR-encoded <json>
	void parse(const char* json, JsonObject& object);

	// Parses the JSONR-encoded <json>
	void parse(Buffer& json, JsonObject& object);
} // namespace JsonRFn

namespace JsonRFn
{
	// Returns the array <json> as Vector2
	// Vector2 = [x, y]
	Vector2 parseVector2(const char* json);

	// Returns the array <json> as Vector3
	// Vector3 = [x, y, z]
	Vector3 parseVector3(const char* json);

	// Returns the array <json> as Vector4.
	// Vector4 = [x, y, z, w]
	Vector4 parseVector4(const char* json);

	// Returns the array <json> as Quaternion.
	// Quaternion = [x, y, z, w]
	Quaternion parseQuaternion(const char* json);

	// Returns the array <json> as Matrix4x4.
	// Matrix4x4 = [xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, tx, ty, tz, tw]
	Matrix4x4 parseMatrix4x4(const char* json);

	// Returns the string <json> as StringId32
	StringId32 parseStringId(const char* json);

	// Returns the string <json> as ResourceId
	ResourceId parseResourceId(const char* json);
} // namespace JsonRFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka