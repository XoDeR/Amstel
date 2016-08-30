// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Json/JsonTypes.h"
#include "Core/Containers/Map.h"

namespace Rio
{
	namespace JsonObjectFn
	{
		// Returns the number of keys
		inline uint32_t getCount(const JsonObject& jsonObject)
		{
			return MapFn::getCount(jsonObject.map);
		}

		// Returns whether the object has the key
		inline bool has(const JsonObject& jsonObject, const char* key)
		{
			return MapFn::has(jsonObject.map, FixedString(key));
		}

		// Returns a pointer to the first item
		inline const Map<FixedString, const char*>::Node* begin(const JsonObject& jsonObject)
		{
			return MapFn::begin(jsonObject.map);
		}

		// Returns a pointer to the item following the last item
		inline const Map<FixedString, const char*>::Node* end(const JsonObject& jsonObject)
		{
			return MapFn::end(jsonObject.map);
		}
	} // namespace JsonObjectFn

	inline JsonObject::JsonObject(Allocator& a)
		: map(a)
	{
	}

	// Returns the value of the <key> or nullptr
	inline const char* JsonObject::operator[](const char* key) const
	{
		return MapFn::get(map, FixedString(key), (const char*)nullptr);
	}

	// Returns the value of the <key> or nullptr
	inline const char* JsonObject::operator[](const FixedString& key) const
	{
		return MapFn::get(map, key, (const char*)nullptr);
	}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka