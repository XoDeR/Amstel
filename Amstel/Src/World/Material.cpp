// Copyright (c) 2016 Volodymyr Syvochka
#include "World/Material.h"
#include "Resource/MaterialResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/TextureResource.h"
#include "World/ShaderManager.h"

#include <bgfx/bgfx.h>

namespace Rio
{

void Material::bind(ResourceManager& resourceManager, ShaderManager& shaderManager, uint8_t view) const
{
	using namespace MaterialResourceFn;

	// Set samplers
	for (uint32_t i = 0; i < materialResource->textureListCount; ++i)
	{
		const TextureData* textureData = getTextureData(materialResource, i);
		const TextureHandle* textureHandle = getTextureHandle(materialResource, i, this->data);

		const TextureResource* textureResource = (TextureResource*)resourceManager.get(RESOURCE_TYPE_TEXTURE, textureData->id);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = textureHandle->samplerHandle;
		texture.idx = textureResource->handle.idx;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < materialResource->uniformListCount; ++i)
	{
		const UniformHandle* uniformHandle = getUniformHandle(materialResource, i, this->data);

		bgfx::UniformHandle bgfxUniformHandle;
		bgfxUniformHandle.idx = uniformHandle->uniformHandle;
		bgfx::setUniform(bgfxUniformHandle, (char*)uniformHandle + sizeof(uniformHandle->uniformHandle));
	}

	const ShaderData& shaderData = shaderManager.get(materialResource->shader);
	bgfx::setState(shaderData.state);
	bgfx::submit(view, shaderData.bgfxProgramHandle);
}

void Material::setFloat(StringId32 name, float value)
{
	char* p = (char*)MaterialResourceFn::getUniformHandleByName(materialResource, name, this->data);
	*(float*)(p + sizeof(uint32_t)) = value;
}

void Material::setVector2(StringId32 name, const Vector2& value)
{
	char* p = (char*)MaterialResourceFn::getUniformHandleByName(materialResource, name, this->data);
	*(Vector2*)(p + sizeof(uint32_t)) = value;
}

void Material::setVector3(StringId32 name, const Vector3& value)
{
	char* p = (char*)MaterialResourceFn::getUniformHandleByName(materialResource, name, this->data);
	*(Vector3*)(p + sizeof(uint32_t)) = value;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka