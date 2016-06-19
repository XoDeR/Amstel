// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Json/JsonR.h"
#include "Core/Base/Macros.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Map.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Math/Quaternion.h"

namespace Rio
{

namespace JsonRFn
{
	static const char* getNext(const char* json, const char c = 0)
	{
		RIO_ASSERT_NOT_NULL(json);

		if (c && c != *json)
		{
			RIO_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skipString(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		bool escaped = false;

		while (*++json)
		{
			if (*json == '"' && !escaped)
			{
				++json;
				return json;
			}
			else if (*json == '\\') escaped = true;
			else escaped = false;
		}

		return json;
	}

	static const char* skipValue(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': json = skipString(json); break;
			case '[': json = skipBlock(json, '[', ']'); break;
			case '{': json = skipBlock(json, '{', '}'); break;
			default: for (; *json != '\0' && *json != ',' && *json != '\n' && *json != ' ' && *json != '}' && *json != ']'; ++json) ; break;
		}

		return json;
	}

	static const char* skipComments(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		if (*json == '/')
		{
			++json;
			if (*json == '/')
			{
				json = getNext(json, '/');
				while (*json && *json != '\n')
				{
					++json;
				}
			}
			else if (*json == '*')
			{
				++json;
				while (*json && *json != '*')
				{
					++json;
				}
				json = getNext(json, '*');
				json = getNext(json, '/');
			}
			else
			{
				RIO_FATAL("Bad comment");
			}
		}

		return json;
	}

	static const char* skipSpaces(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		while (*json)
		{
			if (*json == '/') json = skipComments(json);
			else if (isspace(*json) || *json == ',')
			{
				++json;
			}
			else
			{
				break;
			}
		}

		return json;
	}

	JsonValueType::Enum getType(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': return JsonValueType::STRING;
			case '{': return JsonValueType::OBJECT;
			case '[': return JsonValueType::ARRAY;
			case '-': return JsonValueType::NUMBER;
			default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	void parseString(const char* json, DynamicString& string)
	{
		RIO_ASSERT_NOT_NULL(json);

		if (*json == '"')
		{
			while (*++json)
			{
				// Empty string
				if (*json == '"')
				{
					++json;
					return;
				}
				else if (*json == '\\')
				{
					++json;

					switch (*json)
					{
						case '"': string += '"'; break;
						case '\\': string += '\\'; break;
						case '/': string += '/'; break;
						case 'b': string += '\b'; break;
						case 'f': string += '\f'; break;
						case 'n': string += '\n'; break;
						case 'r': string += '\r'; break;
						case 't': string += '\t'; break;
						default:
						{
							RIO_FATAL("Bad escape character");
							break;
						}
					}
				}
				else
				{
					string += *json;
				}
			}
		}

		RIO_FATAL("Bad string");
	}

	static const char* parseKey(const char* json, DynamicString& key)
	{
		RIO_ASSERT_NOT_NULL(json);
		if (*json == '"')
		{
			parseString(json, key);
			return skipString(json);
		}

		while (true)
		{
			if (isspace(*json) || *json == '=' || *json == ':')
			{
				return json;
			}

			key += *json++;
		}

		RIO_FATAL("Bad key");
		return NULL;
	}

	double parseDouble(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		TempAllocator512 alloc;
	 	Array<char> number(alloc);

		if (*json == '-')
		{
			ArrayFn::pushBack(number, '-');
			++json;
		}
		while (isdigit(*json))
		{
			ArrayFn::pushBack(number, *json);
			++json;
		}

		if (*json == '.')
		{
			ArrayFn::pushBack(number, '.');
			while (*++json && isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			ArrayFn::pushBack(number, *json);
			++json;

			if (*json == '-' || *json == '+')
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
			while (isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
		}

		ArrayFn::pushBack(number, '\0');

		double val;
		int ok = sscanf(ArrayFn::begin(number), "%lf", &val);
		RIO_ASSERT(ok == 1, "Failed to parse double: %s", ArrayFn::begin(number));
		RIO_UNUSED(ok);
		return val;
	}

	bool parseBool(const char* json)
	{
		RIO_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case 't':
			{
				json = getNext(json, 't');
				json = getNext(json, 'r');
				json = getNext(json, 'u');
				json = getNext(json, 'e');
				return true;
			}
			case 'f':
			{
				json = getNext(json, 'f');
				json = getNext(json, 'a');
				json = getNext(json, 'l');
				json = getNext(json, 's');
				json = getNext(json, 'e');
				return false;
			}
			default:
			{
				RIO_FATAL("Bad boolean");
				return false;
			}
		}
	}

	int32_t parseInt(const char* json)
	{
		return (int32_t)parseDouble(json);
	}

	float parseFloat(const char* json)
	{
		return (float)parseDouble(json);
	}

	void parseArray(const char* json, JsonArray& array)
	{
		RIO_ASSERT_NOT_NULL(json);

		if (*json == '[')
		{
			json = skipSpaces(++json);

			if (*json == ']')
			{
				return;
			}

			while (*json)
			{
				ArrayFn::pushBack(array, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == ']')
				{
					return;
				}

				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad array");
	}

	static void parseRootObject(const char* json, JsonObject& object)
	{
		RIO_ASSERT_NOT_NULL(json);

		while (*json)
		{
			const char* keyBegin = *json == '"' ? (json + 1) : json;

			TempAllocator256 ta;
			DynamicString key(ta);
			json = parseKey(json, key);

			FixedString fs_key(keyBegin, key.getLength());

			json = skipSpaces(json);
			json = getNext(json, (*json == '=') ? '=' : ':');
			json = skipSpaces(json);

			MapFn::set(object, fs_key, json);

			json = skipValue(json);
			json = skipSpaces(json);
		}
	}

	void parseObject(const char* json, JsonObject& object)
	{
		RIO_ASSERT_NOT_NULL(json);

		if (*json == '{')
		{
			json = skipSpaces(++json);

			if (*json == '}')
			{
				return;
			}

			while (*json)
			{
				const char* keyBegin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				json = parseKey(json, key);

				FixedString fs_key(keyBegin, key.getLength());

				json = skipSpaces(json);
				json = getNext(json, (*json == '=') ? '=' : ':');
				json = skipSpaces(json);

				MapFn::set(object, fs_key, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == '}')
				{
					return;
				}

				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad object");
	}

	void parse(const char* json, JsonObject& object)
	{
		RIO_ASSERT_NOT_NULL(json);

		json = skipSpaces(json);

		if (*json == '{')
		{
			parseObject(json, object);
		}
		else
		{
			parseRootObject(json, object);
		}
	}

	void parse(Buffer& json, JsonObject& object)
	{
		ArrayFn::pushBack(json, '\0');
		ArrayFn::popBack(json);
		parse(ArrayFn::begin(json), object);
	}
} // namespace JsonRFn

namespace JsonRFn
{
	Vector2 parseVector2(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		JsonRFn::parseArray(json, array);

		Vector2 v;
		v.x = JsonRFn::parseFloat(array[0]);
		v.y = JsonRFn::parseFloat(array[1]);
		return v;
	}

	Vector3 parseVector3(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		JsonRFn::parseArray(json, array);

		Vector3 v;
		v.x = JsonRFn::parseFloat(array[0]);
		v.y = JsonRFn::parseFloat(array[1]);
		v.z = JsonRFn::parseFloat(array[2]);
		return v;
	}

	Vector4 parseVector4(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		JsonRFn::parseArray(json, array);

		Vector4 v;
		v.x = JsonRFn::parseFloat(array[0]);
		v.y = JsonRFn::parseFloat(array[1]);
		v.z = JsonRFn::parseFloat(array[2]);
		v.w = JsonRFn::parseFloat(array[3]);
		return v;
	}

	Quaternion parseQuaternion(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		JsonRFn::parseArray(json, array);

		Quaternion q;
		q.x = JsonRFn::parseFloat(array[0]);
		q.y = JsonRFn::parseFloat(array[1]);
		q.z = JsonRFn::parseFloat(array[2]);
		q.w = JsonRFn::parseFloat(array[3]);
		return q;
	}

	Matrix4x4 parseMatrix4x4(const char* json)
	{
		TempAllocator128 ta;
		JsonArray array(ta);
		JsonRFn::parseArray(json, array);

		Matrix4x4 m;
		m.x.x = JsonRFn::parseFloat(array[0]);
		m.x.y = JsonRFn::parseFloat(array[1]);
		m.x.z = JsonRFn::parseFloat(array[2]);
		m.x.w = JsonRFn::parseFloat(array[3]);

		m.y.x = JsonRFn::parseFloat(array[4]);
		m.y.y = JsonRFn::parseFloat(array[5]);
		m.y.z = JsonRFn::parseFloat(array[6]);
		m.y.w = JsonRFn::parseFloat(array[7]);

		m.z.x = JsonRFn::parseFloat(array[8]);
		m.z.y = JsonRFn::parseFloat(array[9]);
		m.z.z = JsonRFn::parseFloat(array[10]);
		m.z.w = JsonRFn::parseFloat(array[11]);

		m.t.x = JsonRFn::parseFloat(array[12]);
		m.t.y = JsonRFn::parseFloat(array[13]);
		m.t.z = JsonRFn::parseFloat(array[14]);
		m.t.w = JsonRFn::parseFloat(array[15]);
		return m;
	}

	StringId32 parseStringId(const char* json)
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		JsonRFn::parseString(json, str);
		return str.getStringId();
	}

	ResourceId parseResourceId(const char* json)
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		JsonRFn::parseString(json, str);
		return ResourceId(str.getCStr());
	}
} // namespace JsonFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka