// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

struct LevelResource
{
	uint32_t version;
	uint32_t unitsOffset;
	uint32_t soundCount;
	uint32_t soundsOffset;
};

struct LevelSound
{
	StringId64 name;
	Vector3 position;
	float volume;
	float range;
	bool loop;
	char _pad[3];
};

namespace LevelResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace LevelResourceInternalFn

namespace LevelResourceFn
{
	const UnitResource* getUnitResource(const LevelResource* levelResource);
	uint32_t getSoundCount(const LevelResource* levelResource);
	const LevelSound* getSound(const LevelResource* levelResource, uint32_t i);
} // namespace LevelResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka