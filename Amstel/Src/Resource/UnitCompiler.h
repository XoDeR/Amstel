// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Json/JsonTypes.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

class UnitCompiler
{
private:
	using CompileFunction = Buffer (*)(const char* json, CompileOptions& compileOptions);

	struct ComponentTypeData
	{
		ALLOCATOR_AWARE;

		ComponentTypeData(Allocator& a)
			: unitIndexList(a)
			, data(a)
		{
		}

		CompileFunction compileFunction;
		uint32_t componentDataCount = 0;
		Array<uint32_t> unitIndexList;
		Buffer data;
	};

	struct ComponentTypeInfo
	{
		bool operator<(const ComponentTypeInfo& a) const
		{
			return spawnOrder < a.spawnOrder;
		}

		StringId32 unitComponentTypeName;
		float spawnOrder;
	};

	using ComponentTypeDataMap = SortMap<StringId32, ComponentTypeData>;
	using ComponentTypeInfoArray = Array<ComponentTypeInfo>;
	
public:
	UnitCompiler(CompileOptions& compileOptions);

	Buffer readUnit(const char* name);
	void compileUnit(const char* path);
	void compileUnitFromJson(const char* json);
	void compileMultipleUnits(const char* json);

	Buffer getBlob();
private:
	void registerComponentCompiler(const char* type, CompileFunction fn, float spawnOrder);
	void registerComponentCompiler(StringId32 type, CompileFunction fn, float spawnOrder);
	Buffer compileComponent(StringId32 type, const char* json);
	void addComponentData(StringId32 type, const Buffer& data, uint32_t unitIndex);

	CompileOptions& compileOptions;
	uint32_t unitListCount = 0;
	ComponentTypeDataMap componentTypeDataMap;
	ComponentTypeInfoArray componentTypeInfoArray;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka