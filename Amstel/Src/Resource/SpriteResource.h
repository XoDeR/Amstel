// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

// header
// verticesCount
// vertices[verticesCount]
// indicesCount
// indices[indicesCount]

struct SpriteResource
{
	uint32_t version;
	const bgfx::Memory* vertexBufferMemory;
	const bgfx::Memory* indexBufferMemory;
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
};

namespace SpriteResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
} // namespace SpriteResourceInternalFn

struct SpriteAnimationResource
{
	uint32_t version;
	uint32_t frameListCount;
	float totalTime;
};

namespace SpriteAnimationResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
} // SpriteAnimationResourceInternalFn

namespace SpriteAnimationResourceFn
{
	const uint32_t* getAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource);
} // SpriteAnimationResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka