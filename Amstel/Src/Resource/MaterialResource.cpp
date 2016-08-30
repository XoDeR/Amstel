// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/MaterialResource.h"

#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Base/Macros.h"
#include "Core/Containers/Map.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Strings/StringUtils.h"

#include "Device/Device.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#include "World/MaterialManager.h"

namespace Rio
{

namespace MaterialResourceInternalFn
{
	struct UniformTypeInfo
	{
		const char* name;
		UniformType::Enum type;
		uint8_t size;
	};

	static const UniformTypeInfo uniformTypeInfoMap[] =
	{
		{ "float",   UniformType::FLOAT,    4 },
		{ "vector2", UniformType::VECTOR2,  8 },
		{ "vector3", UniformType::VECTOR3, 12 },
		{ "vector4", UniformType::VECTOR4, 16 }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(uniformTypeInfoMap) == UniformType::COUNT);

	static UniformType::Enum getUniformTypeFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(uniformTypeInfoMap); ++i)
		{
			if (strcmp(uniformTypeInfoMap[i].name, name) == 0)
			{
				return uniformTypeInfoMap[i].type;
			}
		}

		return UniformType::COUNT;
	}

	struct Data
	{
		Data()
			: textures(getDefaultAllocator())
			, uniforms(getDefaultAllocator())
			, dynamic(getDefaultAllocator())
		{}

		Array<TextureData> textures;
		Array<UniformData> uniforms;
		Array<char> dynamic;
	};

	// Returns offset to start of data
	template <typename T>
	static uint32_t reserveDynamicData(T data, Array<char>& dynamic)
	{
		uint32_t offset = ArrayFn::getCount(dynamic);
		ArrayFn::push(dynamic, (char*)&data, sizeof(data));
		return offset;
	}

	static void parseTextures(const char* json, Array<TextureData>& textures, Array<char>& names, Array<char>& dynamic, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(json, object);

		auto begin = JsonObjectFn::begin(object);
		auto end = JsonObjectFn::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value = begin->pair.second;

			DynamicString texture(ta);
			JsonRFn::parseString(value, texture);
			RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_TEXTURE, texture.getCStr(), compileOptions);

			TextureHandle textureHandle;
			textureHandle.samplerHandle = 0;
			textureHandle.textureHandle = 0;

			const uint32_t samplerUniformNameOffset = ArrayFn::getCount(names);
			ArrayFn::push(names, key.getData(), key.getLength());
			ArrayFn::pushBack(names, '\0');

			TextureData textureData;
			textureData.samplerUniformNameOffset = samplerUniformNameOffset;
			textureData.id = JsonRFn::parseResourceId(value);
			textureData.dataOffset = reserveDynamicData(textureHandle, dynamic);

			ArrayFn::pushBack(textures, textureData);
		}
	}

	static void parseUniforms(const char* json, Array<UniformData>& uniforms, Array<char>& names, Array<char>& dynamic, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(json, object);

		auto begin = JsonObjectFn::begin(object);
		auto end = JsonObjectFn::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value = begin->pair.second;

			UniformHandle uniformHandle;
			uniformHandle.uniformHandle = 0;

			JsonObject uniform(ta);
			JsonRFn::parseObject(value, uniform);

			DynamicString type(ta);
			JsonRFn::parseString(uniform["type"], type);

			const UniformType::Enum uniformType = getUniformTypeFromName(type.getCStr());
			RESOURCE_COMPILER_ASSERT(uniformType != UniformType::COUNT
				, compileOptions
				, "Unknown uniform type: '%s'"
				, type.getCStr()
				);

			const uint32_t nameOffset = ArrayFn::getCount(names);
			ArrayFn::push(names, key.getData(), key.getLength());
			ArrayFn::pushBack(names, '\0');

			UniformData uniformData;
			uniformData.type = uniformType;
			uniformData.name = StringId32(key.getData(), key.getLength());
			uniformData.nameOffset = nameOffset;
			uniformData.dataOffset = reserveDynamicData(uniformHandle, dynamic);

			switch (uniformData.type)
			{
			case UniformType::FLOAT:
			{
				float data = JsonRFn::parseFloat(uniform["value"]);
				reserveDynamicData(data, dynamic);
			}
			break;
			case UniformType::VECTOR2:
			{
				Vector2 data = JsonRFn::parseVector2(uniform["value"]);
				reserveDynamicData(data, dynamic);
			}
			break;
			case UniformType::VECTOR3:
			{
				Vector3 data = JsonRFn::parseVector3(uniform["value"]);
				reserveDynamicData(data, dynamic);
			}
			break;
			case UniformType::VECTOR4:
			{
				Vector4 data = JsonRFn::parseVector4(uniform["value"]);
				reserveDynamicData(data, dynamic);
			}
			break;
			default: 
			{
				RIO_FATAL("Unknown uniform type");
			}
			break;
			}

			ArrayFn::pushBack(uniforms, uniformData);
		}
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		Array<TextureData> textureDataList(getDefaultAllocator());
		Array<UniformData> uniformDataList(getDefaultAllocator());
		Array<char> names(getDefaultAllocator());
		Array<char> dynamicBlob(getDefaultAllocator());

		DynamicString shaderName(ta);
		JsonRFn::parseString(jsonObject["shader"], shaderName);

		parseTextures(jsonObject["textures"], textureDataList, names, dynamicBlob, compileOptions);
		parseUniforms(jsonObject["uniforms"], uniformDataList, names, dynamicBlob, compileOptions);

		MaterialResource materialResource;
		materialResource.version = RESOURCE_VERSION_MATERIAL;
		materialResource.shader = shaderName.getStringId();
		materialResource.textureListCount = ArrayFn::getCount(textureDataList);
		materialResource.textureDataOffset = sizeof(materialResource);
		materialResource.uniformListCount = ArrayFn::getCount(uniformDataList);
		materialResource.uniformDataOffset = materialResource.textureDataOffset + sizeof(TextureData)*ArrayFn::getCount(textureDataList);
		materialResource.dynamicDataSize = ArrayFn::getCount(dynamicBlob);
		materialResource.dynamicDataOffset = materialResource.uniformDataOffset + sizeof(UniformData)*ArrayFn::getCount(uniformDataList);

		// Write
		compileOptions.write(materialResource.version);
		compileOptions.write(materialResource.shader);
		compileOptions.write(materialResource.textureListCount);
		compileOptions.write(materialResource.textureDataOffset);
		compileOptions.write(materialResource.uniformListCount);
		compileOptions.write(materialResource.uniformDataOffset);
		compileOptions.write(materialResource.dynamicDataSize);
		compileOptions.write(materialResource.dynamicDataOffset);

		for (uint32_t i = 0; i < ArrayFn::getCount(textureDataList); i++)
		{
			compileOptions.write(textureDataList[i].samplerUniformNameOffset);
			compileOptions.write(textureDataList[i]._pad);
			compileOptions.write(textureDataList[i].id);
			compileOptions.write(textureDataList[i].dataOffset);
			compileOptions.write(textureDataList[i]._pad1);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(uniformDataList); i++)
		{
			compileOptions.write(uniformDataList[i].type);
			compileOptions.write(uniformDataList[i].name);
			compileOptions.write(uniformDataList[i].nameOffset);
			compileOptions.write(uniformDataList[i].dataOffset);
		}

		compileOptions.write(dynamicBlob);
		compileOptions.write(names);
	}

	void* load(File& file, Allocator& a)
	{
		return getDevice()->getMaterialManager()->load(file, a);
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->getMaterialManager()->online(id, resourceManager);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->getMaterialManager()->offline(id, resourceManager);
	}

	void unload(Allocator& a, void* resource)
	{
		getDevice()->getMaterialManager()->unload(a, resource);
	}

} // namespace MaterialResourceInternalFn

namespace MaterialResourceFn
{
	UniformData* getUniformData(const MaterialResource* materialResource, uint32_t i)
	{
		UniformData* base = (UniformData*) ((char*)materialResource + materialResource->uniformDataOffset);
		return &base[i];
	}

	UniformData* getUniformDataByName(const MaterialResource* materialResource, StringId32 name)
	{
		for (uint32_t i = 0, n = materialResource->uniformListCount; i < n; ++i)
		{
			UniformData* data = getUniformData(materialResource, i);
			if (data->name == name)
			{
				return data;
			}
		}

		RIO_FATAL("Unknown uniform");
		return nullptr;
	}

	const char* getUniformName(const MaterialResource* materialResource, const UniformData* uniformData)
	{
		return (const char*)materialResource + materialResource->dynamicDataOffset + materialResource->dynamicDataSize + uniformData->nameOffset;
	}

	TextureData* getTextureData(const MaterialResource* materialResource, uint32_t i)
	{
		TextureData* base = (TextureData*) ((char*)materialResource + materialResource->textureDataOffset);
		return &base[i];
	}

	const char* getTextureName(const MaterialResource* materialResource, const TextureData* textureData)
	{
		return (const char*)materialResource + materialResource->dynamicDataOffset + materialResource->dynamicDataSize + textureData->samplerUniformNameOffset;
	}

	UniformHandle* getUniformHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic)
	{
		UniformData* uniformData = getUniformData(materialResource, i);
		return (UniformHandle*) (dynamic + uniformData->dataOffset);
	}

	UniformHandle* getUniformHandleByName(const MaterialResource* materialResource, StringId32 name, char* dynamic)
	{
		UniformData* uniformData = getUniformDataByName(materialResource, name);
		return (UniformHandle*) (dynamic + uniformData->dataOffset);
	}

	TextureHandle* getTextureHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic)
	{
		TextureData* textureData = getTextureData(materialResource, i);
		return (TextureHandle*) (dynamic + textureData->dataOffset);
	}
} // namespace MaterialResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka