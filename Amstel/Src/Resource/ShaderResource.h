// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Containers/ContainerTypes.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

struct ShaderResource
{
	struct Data
	{
		StringId32 name;
		uint64_t state;
		const bgfx::Memory* vertexShaderMemory;
		const bgfx::Memory* fragmentShaderMemory;
	};

	explicit ShaderResource(Allocator& a)
		: data(a)
	{
	}

	Array<Data> data;
};

namespace ShaderResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
} // namespace ShaderResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka