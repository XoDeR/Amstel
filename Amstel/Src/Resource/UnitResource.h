// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/CompilerTypes.h"
#include <cstdint> // uint32_t

namespace Rio
{

struct UnitResource
{
	uint32_t version;
	uint32_t unitListCount;
	uint32_t componentTypesCount;
//	ComponentData data[componentTypesCount]
};

struct ComponentData
{
	StringId32 type;
	// entity
	uint32_t instancesCount;
	uint32_t size;
//	uint32_t unitIndexList[instancesCount]
//	char data[size]
};

namespace UnitResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace UnitResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka