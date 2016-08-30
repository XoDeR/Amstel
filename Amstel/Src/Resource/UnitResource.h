// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

struct UnitResource
{
	uint32_t version = 0;
	uint32_t unitListCount = 0;
	uint32_t componentTypesCount = 0;
//	ComponentData data[componentTypesCount]
};

struct ComponentData
{
	StringId32 type;
	// entity
	uint32_t instancesCount = 0;
	uint32_t size = 0;
//	uint32_t unitIndexList[instancesCount]
//	char data[size]
};

namespace UnitResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace UnitResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka