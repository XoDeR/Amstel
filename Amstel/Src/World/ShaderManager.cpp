// Copyright (c) 2016 Volodymyr Syvochka
#include "World/ShaderManager.h"
#include "Core/Containers/Array.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Containers/SortMap.h"
#include "Core/Strings/StringUtils.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderResource.h"

namespace Rio
{

ShaderManager::ShaderManager(Allocator& a)
	: shaderMap(a)
{
}

void* ShaderManager::load(File& file, Allocator& a)
{
	BinaryReader binaryReader(file);
	uint32_t version;
	binaryReader.read(version);
	RIO_ASSERT(version == RESOURCE_VERSION_SHADER, "Wrong version");

	uint32_t shaderListCount;
	binaryReader.read(shaderListCount);

	ShaderResource* shaderResource = RIO_NEW(a, ShaderResource)(a);
	ArrayFn::resize(shaderResource->data, shaderListCount);

	for (uint32_t i = 0; i < shaderListCount; ++i)
	{
		uint32_t shaderNameHash;
		binaryReader.read(shaderNameHash);

		uint64_t renderState;
		binaryReader.read(renderState);

		uint32_t vertexShaderCodeSize;
		binaryReader.read(vertexShaderCodeSize);
		const bgfx::Memory* vertexShaderMemory = bgfx::alloc(vertexShaderCodeSize);
		binaryReader.read(vertexShaderMemory->data, vertexShaderCodeSize);

		uint32_t fragmentShaderCodeSize;
		binaryReader.read(fragmentShaderCodeSize);
		const bgfx::Memory* fragmentShaderMemory = bgfx::alloc(fragmentShaderCodeSize);
		binaryReader.read(fragmentShaderMemory->data, fragmentShaderCodeSize);

		shaderResource->data[i].name.id = shaderNameHash;
		shaderResource->data[i].state = renderState;
		shaderResource->data[i].vertexShaderMemory = vertexShaderMemory;
		shaderResource->data[i].fragmentShaderMemory = fragmentShaderMemory;
	}

	return shaderResource;
}

void ShaderManager::online(StringId64 id, ResourceManager& resourceManager)
{
	const ShaderResource* shader = (ShaderResource*)resourceManager.get(RESOURCE_TYPE_SHADER, id);

	for (uint32_t i = 0; i < ArrayFn::getCount(shader->data); ++i)
	{
		const ShaderResource::Data& data = shader->data[i];

		bgfx::ShaderHandle vertexShaderHandle = bgfx::createShader(data.vertexShaderMemory);
		RIO_ASSERT(bgfx::isValid(vertexShaderHandle), "Failed to create vertex shader");
		bgfx::ShaderHandle fragmentShaderHandle = bgfx::createShader(data.fragmentShaderMemory);
		RIO_ASSERT(bgfx::isValid(fragmentShaderHandle), "Failed to create fragment shader");
		bgfx::ProgramHandle bgfxProgramHandle = bgfx::createProgram(vertexShaderHandle, fragmentShaderHandle, true);
		RIO_ASSERT(bgfx::isValid(bgfxProgramHandle), "Failed to create GPU program");

		addShader(data.name, data.state, bgfxProgramHandle);
	}
}

void ShaderManager::offline(StringId64 id, ResourceManager& resourceManager)
{
	const ShaderResource* shader = (ShaderResource*)resourceManager.get(RESOURCE_TYPE_SHADER, id);

	for (uint32_t i = 0; i < ArrayFn::getCount(shader->data); ++i)
	{
		const ShaderResource::Data& data = shader->data[i];
		const ShaderData& shaderData = get(data.name);

		bgfx::destroyProgram(shaderData.bgfxProgramHandle);

		SortMapFn::remove(shaderMap, data.name);
		SortMapFn::sort(shaderMap);
	}
}

void ShaderManager::unload(Allocator& a, void* resource)
{
	RIO_DELETE(a, (ShaderResource*)resource);
}

void ShaderManager::addShader(StringId32 name, uint64_t state, bgfx::ProgramHandle bgfxProgramHandle)
{
	ShaderData shaderData;
	shaderData.state = state;
	shaderData.bgfxProgramHandle = bgfxProgramHandle;
	SortMapFn::set(shaderMap, name, shaderData);
	SortMapFn::sort(shaderMap);
}

const ShaderData& ShaderManager::get(StringId32 id)
{
	RIO_ASSERT(SortMapFn::has(shaderMap, id), "Shader not found");
	ShaderData deffault;
	deffault.state = BGFX_STATE_DEFAULT;
	deffault.bgfxProgramHandle = BGFX_INVALID_HANDLE;
	return SortMapFn::get(shaderMap, id, deffault);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka