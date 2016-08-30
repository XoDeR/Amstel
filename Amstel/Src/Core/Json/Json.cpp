// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Json/Json.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Base/Macros.h"
#include "Core/Containers/Map.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

namespace JsonFn
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
			else if (*json == '\\')
			{
				escaped = true;
			}
			else
			{
				escaped = false;
			}
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
			default: for (; *json != ',' && *json != '}' && *json != ']'; ++json) ; break;
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

				json = getNext(json, ',');
				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad array");
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
				parseString(json, key);

				FixedString fixedStringKey(keyBegin, key.getLength());

				json = skipString(json);
				json = skipSpaces(json);
				json = getNext(json, ':');
				json = skipSpaces(json);

				MapFn::set(object.map, fixedStringKey, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == '}')
				{
					return;
				}

				json = getNext(json, ',');
				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad object");
	}

	void parse(const char* json, JsonObject& object)
	{
		RIO_ASSERT_NOT_NULL(json);
		parseObject(json, object);
	}

	void parse(Buffer& json, JsonObject& object)
	{
		ArrayFn::pushBack(json, '\0');
		ArrayFn::popBack(json);
		parse(ArrayFn::begin(json), object);
	}
} // namespace JsonFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka