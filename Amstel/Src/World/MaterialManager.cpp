// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/FileSystem/File.h"
#include "Core/Containers/SortMap.h"
#include "Resource/MaterialResource.h"
#include "Resource/ResourceManager.h"
#include "World/MaterialManager.h"

#include <string.h> // memcpy
#include <bgfx/bgfx.h>

namespace Rio
{

MaterialManager::MaterialManager(Allocator& a, ResourceManager& resourceManager)
	: allocator(&a)
	, resourceManager(&resourceManager)
	, materialsMap(a)
{
}

MaterialManager::~MaterialManager()
{
	auto begin = SortMapFn::begin(materialsMap);
	auto end = SortMapFn::end(materialsMap);
	for (; begin != end; ++begin)
	{
		allocator->deallocate(begin->pair.second);
	}
}

void* MaterialManager::load(File& file, Allocator& a)
{
	const uint32_t fileSize = file.getSize();
	void* resource = a.allocate(fileSize);
	file.read(resource, fileSize);
	RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_MATERIAL, "Wrong version");
	return resource;
}

void MaterialManager::online(StringId64 id, ResourceManager& resourceManager)
{
	using namespace MaterialResourceFn;

	MaterialResource* materialResource = (MaterialResource*)resourceManager.get(RESOURCE_TYPE_MATERIAL, id);

	char* base = (char*)materialResource + materialResource->dynamicDataOffset;

	for (uint32_t i = 0; i < materialResource->textureListCount; ++i)
	{
		TextureData* textureData = getTextureData(materialResource, i);
		TextureHandle* textureHandle = getTextureHandle(materialResource, i, base);
		textureHandle->samplerHandle = bgfx::createUniform(getTextureName(materialResource, textureData), bgfx::UniformType::Int1).idx;
	}

	for (uint32_t i = 0; i < materialResource->uniformListCount; ++i)
	{
		UniformData* uniformData = getUniformData(materialResource, i);
		UniformHandle* uniformHandle = getUniformHandle(materialResource, i, base);
		uniformHandle->uniformHandle = bgfx::createUniform(getUniformName(materialResource, uniformData), bgfx::UniformType::Vec4).idx;
	}
}

void MaterialManager::offline(StringId64 id, ResourceManager& resourceManager)
{
	using namespace MaterialResourceFn;

	MaterialResource* materialResource = (MaterialResource*)resourceManager.get(RESOURCE_TYPE_MATERIAL, id);

	char* base = (char*)materialResource + materialResource->dynamicDataOffset;

	for (uint32_t i = 0; i < materialResource->textureListCount; ++i)
	{
		TextureHandle* textureHandle = getTextureHandle(materialResource, i, base);
		bgfx::UniformHandle bgfxUniformHandle;
		bgfxUniformHandle.idx = textureHandle->samplerHandle;
		bgfx::destroyUniform(bgfxUniformHandle);
	}

	for (uint32_t i = 0; i < materialResource->uniformListCount; ++i)
	{
		UniformHandle* uniformHandle = getUniformHandle(materialResource, i, base);
		bgfx::UniformHandle bgfxUniformHandle;
		bgfxUniformHandle.idx = uniformHandle->uniformHandle;
		bgfx::destroyUniform(bgfxUniformHandle);
	}
}

void MaterialManager::unload(Allocator& a, void* resource)
{
	a.deallocate(resource);
}

void MaterialManager::createMaterial(StringId64 id)
{
	if (SortMapFn::has(materialsMap, id))
	{
		return;
	}

	const MaterialResource* materialResource = (MaterialResource*)resourceManager->get(RESOURCE_TYPE_MATERIAL, id);

	const uint32_t size = sizeof(Material) + materialResource->dynamicDataSize;
	Material* material = (Material*)allocator->allocate(size);
	material->materialResource = materialResource;
	material->data = (char*)&material[1];

	const char* data = (char*)materialResource + materialResource->dynamicDataOffset;
	memcpy(material->data, data, materialResource->dynamicDataSize);

	SortMapFn::set(materialsMap, id, material);
	SortMapFn::sort(materialsMap);
}

void MaterialManager::destroyMaterial(StringId64 id)
{
	Material* material = SortMapFn::get(materialsMap, id, (Material*)nullptr);
	allocator->deallocate(material);

	SortMapFn::remove(materialsMap, id);
	SortMapFn::sort(materialsMap);
}

Material* MaterialManager::get(StringId64 id)
{
	RIO_ASSERT(SortMapFn::has(materialsMap, id), "Material not found");
	return SortMapFn::get(materialsMap, id, (Material*)nullptr);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka