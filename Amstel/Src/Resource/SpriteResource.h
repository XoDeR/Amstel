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

namespace SpriteResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
} // namespace SpriteResourceFn

struct SpriteAnimationResource
{
	uint32_t version;
	uint32_t animationListCount;
	uint32_t frameListCount;
	uint32_t framesOffset;
};

struct SpriteAnimationName
{
	StringId32 id;
};

struct SpriteAnimationData
{
	uint32_t frameListCount;
	uint32_t firstFrameIndex;
	float time;
};

namespace SpriteAnimationResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
	const SpriteAnimationData* getSpriteAnimationData(const SpriteAnimationResource* spriteAnimationResource, StringId32 name);
	const uint32_t* getAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka