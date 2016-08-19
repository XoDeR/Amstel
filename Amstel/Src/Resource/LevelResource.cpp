// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/LevelResource.h"
#include "Core/Containers/Array.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Containers/Map.h"
#include "Core/Memory/Memory.h"
#include "Core/Json/JsonR.h"
#include "Resource/CompileOptions.h"
#include "Resource/UnitCompiler.h"

namespace Rio
{

namespace LevelResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		Array<LevelSound> sounds(getDefaultAllocator());
		{
			JsonObject soundsJson(ta);
			JsonRFn::parseObject(jsonObject["sounds"], soundsJson);

			auto begin = MapFn::begin(soundsJson);
			auto end = MapFn::end(soundsJson);
			for (; begin != end; ++begin)
			{
				JsonObject sound(ta);
				JsonRFn::parseObject(begin->pair.second, sound);

				LevelSound levelSound;
				levelSound.name = JsonRFn::parseResourceId(sound["name"]);
				levelSound.position = JsonRFn::parseVector3(sound["position"]);
				levelSound.volume = JsonRFn::parseFloat(sound["volume"]);
				levelSound.range = JsonRFn::parseFloat(sound["range"]);
				levelSound.loop = JsonRFn::parseBool(sound["loop"]);

				ArrayFn::pushBack(sounds, levelSound);
			}
		}

		UnitCompiler unitCompiler(compileOptions);
		unitCompiler.compileMultipleUnits(jsonObject["units"]);
		Buffer unitBlob = unitCompiler.getBlob();

		// Write
		LevelResource levelResource;
		levelResource.version = RESOURCE_VERSION_LEVEL;
		levelResource.soundCount = ArrayFn::getCount(sounds);
		levelResource.unitsOffset  = sizeof(LevelResource);
		levelResource.soundsOffset = levelResource.unitsOffset + ArrayFn::getCount(unitBlob);

		compileOptions.write(levelResource.version);
		compileOptions.write(levelResource.unitsOffset);
		compileOptions.write(levelResource.soundCount);
		compileOptions.write(levelResource.soundsOffset);

		compileOptions.write(unitBlob);

		for (uint32_t i = 0; i < ArrayFn::getCount(sounds); ++i)
		{
			compileOptions.write(sounds[i].name);
			compileOptions.write(sounds[i].position);
			compileOptions.write(sounds[i].volume);
			compileOptions.write(sounds[i].range);
			compileOptions.write(sounds[i].loop);
			compileOptions.write(sounds[i]._pad[0]);
			compileOptions.write(sounds[i]._pad[1]);
			compileOptions.write(sounds[i]._pad[2]);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t fileSize = file.getSize();
		void* result = a.allocate(fileSize);
		file.read(result, fileSize);
		RIO_ASSERT(*(uint32_t*)result == RESOURCE_VERSION_LEVEL, "Wrong version");
		return result;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const UnitResource* getUnitResource(const LevelResource* levelResource)
	{
		return (const UnitResource*)((char*)levelResource + levelResource->unitsOffset);
	}

	uint32_t getSoundCount(const LevelResource* levelResource)
	{
		return levelResource->soundCount;
	}

	const LevelSound* getSound(const LevelResource* levelResource, uint32_t i)
	{
		RIO_ASSERT(i < getSoundCount(levelResource), "Index out of bounds");
		const LevelSound* begin = (LevelSound*)((char*)levelResource + levelResource->soundsOffset);
		return &begin[i];
	}
} // namespace LevelResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka