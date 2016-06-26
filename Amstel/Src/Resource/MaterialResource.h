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

struct MaterialResource
{
	uint32_t version;
	StringId32 shader;
	uint32_t textureListCount;
	uint32_t textureDataOffset;
	uint32_t uniformListCount;
	uint32_t uniformDataOffset;
	uint32_t dynamicDataSize;
	uint32_t dynamicDataOffset;
};

struct TextureData
{
	uint32_t samplerUniformNameOffset; // Sampler uniform name
	uint32_t _pad;
	StringId64 id; // Resource name
	uint32_t dataOffset;  // Offset into dynamic blob
	uint32_t _pad1;
};

struct TextureHandle
{
	uint32_t samplerHandle;
	uint32_t textureHandle;
};

struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,
		COUNT
	};
};

struct UniformData
{
	uint32_t type; // UniformType::Enum
	StringId32 name; // Uniform name
	uint32_t nameOffset; // Uniform name (string)
	uint32_t dataOffset; // Offset into dynamic blob
};

struct UniformHandle
{
	uint32_t uniformHandle;
	// data
};

namespace MaterialResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);

	UniformData* getUniformData(const MaterialResource* materialResource, uint32_t i);
	UniformData* getUniformDataByName(const MaterialResource* materialResource, StringId32 name);
	const char* getUniformName(const MaterialResource* materialResource, const UniformData* uniformData);
	TextureData* getTextureData(const MaterialResource* materialResource, uint32_t i);
	const char* getTextureName(const MaterialResource* materialResource, const TextureData* textureData);
	UniformHandle* getUniformHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic);
	UniformHandle* getUniformHandleByName(const MaterialResource* materialResource, StringId32 name, char* dynamic);
	TextureHandle* getTextureHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic);
} // namespace MaterialResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka