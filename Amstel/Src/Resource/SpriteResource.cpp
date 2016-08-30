// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/SpriteResource.h"

#include "Config.h"

#include "Core/Strings/StringUtils.h"
#include "Core/Containers/Array.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"

#include "Resource/ResourceManager.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

namespace SpriteResourceInternalFn
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region;  // [x, y, w, h]
		Vector2 pivot;  // [x, y]
	};

	void parseFrame(const char* json, SpriteFrame& frame)
	{
		TempAllocator512 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		frame.name   = JsonRFn::parseStringId(jsonObject["name"]);
		frame.region = JsonRFn::parseVector4(jsonObject["region"]);
		frame.pivot = JsonRFn::parseVector2(jsonObject["pivot"]);
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(buffer, object);

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
			const float v0 = (spriteFrame.region.y + spriteFrame.region.w) / height;
			const float u1 = (spriteFrame.region.x + spriteFrame.region.z) / width;
			const float v1 = spriteFrame.region.y / height;

			// Compute positions
			float x0 = spriteFrame.region.x - spriteFrame.pivot.x;
			float y0 = -(spriteFrame.region.y + spriteFrame.region.w - spriteFrame.pivot.y);
			float x1 = spriteFrame.region.x + spriteFrame.region.z - spriteFrame.pivot.x;
			float y1 = -(spriteFrame.region.y - spriteFrame.pivot.y);
			x0 /= RIO_DEFAULT_PIXELS_PER_METER;
			y0 /= RIO_DEFAULT_PIXELS_PER_METER;
			x1 /= RIO_DEFAULT_PIXELS_PER_METER;
			y1 /= RIO_DEFAULT_PIXELS_PER_METER;

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
} // namespace SpriteResourceInternalFn

namespace SpriteAnimationResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonArray objectFrames(ta);

		Array<uint32_t> frames(getDefaultAllocator());
		float totalTime = 0.0f;

		JsonRFn::parse(buffer, jsonObject);
		JsonRFn::parseArray(jsonObject["frames"], objectFrames);

		ArrayFn::resize(frames, ArrayFn::getCount(objectFrames));
		for (uint32_t i = 0; i < ArrayFn::getCount(objectFrames); ++i)
		{
			frames[i] = (uint32_t)JsonRFn::parseFloat(objectFrames[i]);
		}

		totalTime = JsonRFn::parseFloat(jsonObject["totalTime"]);

		// Write
		SpriteAnimationResource spriteAnimationResource;
		spriteAnimationResource.version = RESOURCE_VERSION_SPRITE_ANIMATION;
		spriteAnimationResource.frameListCount = ArrayFn::getCount(frames);
		spriteAnimationResource.totalTime = totalTime;

		compileOptions.write(spriteAnimationResource.version);
		compileOptions.write(spriteAnimationResource.frameListCount);
		compileOptions.write(spriteAnimationResource.totalTime);

		for (uint32_t i = 0; i < ArrayFn::getCount(frames); ++i)
		{
			compileOptions.write(frames[i]);
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
} // namespace SpriteAnimationResourceInternalFn

namespace SpriteAnimationResourceFn
{
	const uint32_t* getAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource)
	{
		return (uint32_t*)&spriteAnimationResource[1];
	}
} // namespace SpriteAnimationResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka