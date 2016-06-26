// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/SpriteResource.h"
#include "Config.h"
#include "Core/Memory/Allocator.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Json/JsonR.h"
#include "Resource/ResourceManager.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

namespace SpriteResourceFn
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region;  // [x0, y0, x1, y1]
		Vector2 scale;   // [Sx, Sy]
		Vector2 offset;  // [Ox, Oy]
	};

	void parseFrame(const char* json, SpriteFrame& frame)
	{
		TempAllocator512 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		frame.name   = JsonRFn::parseStringId(jsonObject["name"]);
		frame.region = JsonRFn::parseVector4(jsonObject["region"]);
		frame.offset = JsonRFn::parseVector2(jsonObject["offset"]);
		frame.scale  = JsonRFn::parseVector2(jsonObject["scale"]);
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buf = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(buf, object);

		JsonArray frames(ta);
		JsonRFn::parseArray(object["frames"], frames);

		// Read width/height
		const float width = JsonRFn::parseFloat(object["width" ]);
		const float height = JsonRFn::parseFloat(object["height"]);
		const uint32_t frameListCount = ArrayFn::getCount(frames);

		Array<float> vertices(getDefaultAllocator());
		Array<uint16_t> indices(getDefaultAllocator());
		uint32_t currentIndex = 0;
		for (uint32_t i = 0; i < frameListCount; ++i)
		{
			SpriteFrame spriteFrameParsed;
			parseFrame(frames[i], spriteFrameParsed);

			const SpriteFrame& spriteFrame = spriteFrameParsed;

			// Compute uv coords
			const float u0 = spriteFrame.region.x / width;
			const float v0 = spriteFrame.region.y / height;
			const float u1 = (spriteFrame.region.x + spriteFrame.region.z) / width;
			const float v1 = (spriteFrame.region.y + spriteFrame.region.w) / height;

			// Compute positions
			const float w = spriteFrame.region.z / RIO_DEFAULT_PIXELS_PER_METER;
			const float h = spriteFrame.region.w / RIO_DEFAULT_PIXELS_PER_METER;

			const float x0 = spriteFrame.scale.x * (-w * 0.5f) + spriteFrame.offset.x;
			const float y0 = spriteFrame.scale.y * (-h * 0.5f) + spriteFrame.offset.y;
			const float x1 = spriteFrame.scale.x * ( w * 0.5f) + spriteFrame.offset.x;
			const float y1 = spriteFrame.scale.y * ( h * 0.5f) + spriteFrame.offset.y;

			ArrayFn::pushBack(vertices, x0); ArrayFn::pushBack(vertices, y0); // position
			ArrayFn::pushBack(vertices, u0); ArrayFn::pushBack(vertices, v0); // uv

			ArrayFn::pushBack(vertices, x1); ArrayFn::pushBack(vertices, y0); // position
			ArrayFn::pushBack(vertices, u1); ArrayFn::pushBack(vertices, v0); // uv

			ArrayFn::pushBack(vertices, x1); ArrayFn::pushBack(vertices, y1); // position
			ArrayFn::pushBack(vertices, u1); ArrayFn::pushBack(vertices, v1); // uv

			ArrayFn::pushBack(vertices, x0); ArrayFn::pushBack(vertices, y1); // position
			ArrayFn::pushBack(vertices, u0); ArrayFn::pushBack(vertices, v1); // uv

			ArrayFn::pushBack(indices, uint16_t(currentIndex));
			ArrayFn::pushBack(indices, uint16_t(currentIndex + 1));
			ArrayFn::pushBack(indices, uint16_t(currentIndex + 2));

			ArrayFn::pushBack(indices, uint16_t(currentIndex)); 
			ArrayFn::pushBack(indices, uint16_t(currentIndex + 2));
			ArrayFn::pushBack(indices, uint16_t(currentIndex + 3));
			currentIndex += 4;
		}

		const uint32_t verticesCount = ArrayFn::getCount(vertices) / 4; // 4 components per vertex
		const uint32_t indicesCount = ArrayFn::getCount(indices);

		// Write
		compileOptions.write(RESOURCE_VERSION_SPRITE);

		compileOptions.write(verticesCount);
		for (uint32_t i = 0; i < ArrayFn::getCount(vertices); i++)
		{
			compileOptions.write(vertices[i]);
		}

		compileOptions.write(indicesCount);
		for (uint32_t i = 0; i < ArrayFn::getCount(indices); i++)
		{
			compileOptions.write(indices[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);

		uint32_t verticesCount;
		binaryReader.read(verticesCount);
		const bgfx::Memory* vertexBufferMemory = bgfx::alloc(verticesCount * sizeof(float) * 4);
		binaryReader.read(vertexBufferMemory->data, verticesCount * sizeof(float) * 4);

		uint32_t indicesCount;
		binaryReader.read(indicesCount);
		const bgfx::Memory* indexBufferMemory = bgfx::alloc(indicesCount * sizeof(uint16_t));
		binaryReader.read(indexBufferMemory->data, indicesCount * sizeof(uint16_t));

		SpriteResource* spriteResource = (SpriteResource*) a.allocate(sizeof(SpriteResource));
		spriteResource->vertexBufferMemory = vertexBufferMemory;
		spriteResource->indexBufferMemory = indexBufferMemory;

		return spriteResource;
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		SpriteResource* spriteResource = (SpriteResource*)resourceManager.get(RESOURCE_TYPE_SPRITE, id);

		bgfx::VertexDecl vertexDecl;
		vertexDecl.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false)
			.end();

		spriteResource->vertexBufferHandle = bgfx::createVertexBuffer(spriteResource->vertexBufferMemory, vertexDecl);
		spriteResource->indexBufferHandle = bgfx::createIndexBuffer(spriteResource->indexBufferMemory);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		SpriteResource* spriteResource = (SpriteResource*)resourceManager.get(RESOURCE_TYPE_SPRITE, id);

		bgfx::destroyVertexBuffer(spriteResource->vertexBufferHandle);
		bgfx::destroyIndexBuffer(spriteResource->indexBufferHandle);
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}
} // namespace SpriteResourceFn

namespace SpriteAnimationResourceFn
{
	void parseAnimation(const char* json, Array<SpriteAnimationName>& spriteAnimationNameList, Array<SpriteAnimationData>& spriteAnimationDataList, Array<uint32_t>& frames)
	{
		TempAllocator512 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		SpriteAnimationName spriteAnimationName;
		spriteAnimationName.id = JsonRFn::parseStringId(jsonObject["name"]);

		JsonArray frameJsonObjectList(ta);
		JsonRFn::parseArray(jsonObject["frames"], frameJsonObjectList);

		const uint32_t frameListCount = ArrayFn::getCount(frameJsonObjectList);

		SpriteAnimationData spriteAnimationData;
		spriteAnimationData.frameListCount = frameListCount;
		spriteAnimationData.firstFrameIndex = ArrayFn::getCount(frames);
		spriteAnimationData.time = JsonRFn::parseFloat(jsonObject["time"]);

		// Read frames
		for (uint32_t frameIndex = 0; frameIndex < frameListCount; ++frameIndex)
		{
			ArrayFn::pushBack(frames, (uint32_t)JsonRFn::parseInt(frameJsonObjectList[frameIndex]));
		}

		ArrayFn::pushBack(spriteAnimationNameList, spriteAnimationName);
		ArrayFn::pushBack(spriteAnimationDataList, spriteAnimationData);
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		JsonArray animations(ta);
		JsonRFn::parseArray(jsonObject["animations"], animations);

		Array<SpriteAnimationName> spriteAnimationNameList(getDefaultAllocator());
		Array<SpriteAnimationData> spriteAnimationDataList(getDefaultAllocator());
		Array<uint32_t> spriteAnimationFrameIndexList(getDefaultAllocator());

		const uint32_t animationListCount = ArrayFn::getCount(animations);
		for (uint32_t i = 0; i < animationListCount; ++i)
		{
			parseAnimation(animations[i], spriteAnimationNameList, spriteAnimationDataList, spriteAnimationFrameIndexList);
		}

		SpriteAnimationResource spriteAnimationResource;
		spriteAnimationResource.version = RESOURCE_VERSION_SPRITE_ANIMATION;
		spriteAnimationResource.animationListCount = ArrayFn::getCount(spriteAnimationNameList);
		spriteAnimationResource.frameListCount = ArrayFn::getCount(spriteAnimationFrameIndexList);
		spriteAnimationResource.framesOffset = uint32_t(sizeof(SpriteAnimationResource) +
					sizeof(SpriteAnimationName) * ArrayFn::getCount(spriteAnimationNameList) +
					sizeof(SpriteAnimationData) * ArrayFn::getCount(spriteAnimationDataList));

		compileOptions.write(spriteAnimationResource.version);
		compileOptions.write(spriteAnimationResource.animationListCount);
		compileOptions.write(spriteAnimationResource.frameListCount);
		compileOptions.write(spriteAnimationResource.framesOffset);

		for (uint32_t i = 0; i < ArrayFn::getCount(spriteAnimationNameList); i++)
		{
			compileOptions.write(spriteAnimationNameList[i].id);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(spriteAnimationDataList); i++)
		{
			compileOptions.write(spriteAnimationDataList[i].frameListCount);
			compileOptions.write(spriteAnimationDataList[i].firstFrameIndex);
			compileOptions.write(spriteAnimationDataList[i].time);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(spriteAnimationFrameIndexList); i++)
		{
			compileOptions.write(spriteAnimationFrameIndexList[i]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t fileSize = file.getSize();
		void* resource = a.allocate(fileSize);
		file.read(resource, fileSize);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_SPRITE, "Wrong version");
		return resource;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

	const SpriteAnimationData* getSpriteAnimationData(const SpriteAnimationResource* spriteAnimationResource, StringId32 name)
	{
		const uint32_t animationListCount = spriteAnimationResource->animationListCount;
		const SpriteAnimationName* begin = (SpriteAnimationName*) ((char*)spriteAnimationResource + sizeof(*spriteAnimationResource));
		const SpriteAnimationData* data = (SpriteAnimationData*) ((char*)spriteAnimationResource + sizeof(*spriteAnimationResource) + sizeof(SpriteAnimationName) * animationListCount);

		for (uint32_t i = 0; i < animationListCount; i++)
		{
			if (begin[i].id == name)
			{
				return &data[i];
			}
		}

		return nullptr;
	}

	const uint32_t* getAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource)
	{
		return (uint32_t*) ((char*)spriteAnimationResource + spriteAnimationResource->framesOffset);
	}
} // namespace SpriteAnimationResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka