// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

struct TextureResource
{
	const bgfx::Memory* memoryBuffer;
	bgfx::TextureHandle handle;
};

namespace TextureResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void online(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
} // namespace TextureResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka