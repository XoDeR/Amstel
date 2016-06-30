// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Resource/ResourceTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

struct ShaderData
{
	uint64_t state;
	bgfx::ProgramHandle bgfxProgramHandle;
};

class ShaderManager
{
private:
	using ShaderMap = SortMap<StringId32, ShaderData>;
public:
	ShaderManager(Allocator& a);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
	const ShaderData& get(StringId32 id);
private:
	void addShader(StringId32 name, uint64_t state, bgfx::ProgramHandle bgfxProgramHandle);
	ShaderMap shaderMap;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka