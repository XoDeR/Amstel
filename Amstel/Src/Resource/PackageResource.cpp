// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/PackageResource.h"
#include "Core/Containers/Array.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonR.h"
#include "Core/Strings/StringId.h"
#include "Core/Memory/TempAllocator.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

namespace PackageResourceFn
{
	void compileResources(const char* type, const JsonArray& names, Array<PackageResource::Resource>& output, CompileOptions& compileOptions)
	{
		const StringId64 typeHash = StringId64(type);

		for (uint32_t i = 0; i < ArrayFn::getCount(names); ++i)
		{
			TempAllocator1024 ta;
			DynamicString name(ta);
			JsonRFn::parseString(names[i], name);

			RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(type, name.getCStr(), compileOptions);

			const StringId64 nameHash = JsonRFn::parseResourceId(names[i]);
			ArrayFn::pushBack(output, PackageResource::Resource(typeHash, nameHash));
		}
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(buffer, object);

		JsonArray texture(ta);
		JsonArray script(ta);
		JsonArray sound(ta);
		JsonArray mesh(ta);
		JsonArray unit(ta);
		JsonArray sprite(ta);
		JsonArray material(ta);
		JsonArray font(ta);
		JsonArray level(ta);
		JsonArray physicsConfig(ta);
		JsonArray shader(ta);
		JsonArray spriteAnimation(ta);

		if (MapFn::has(object, FixedString("texture"))) JsonRFn::parseArray(object["texture"], texture);
		if (MapFn::has(object, FixedString("shader"))) JsonRFn::parseArray(object["shader"], shader);
		if (MapFn::has(object, FixedString("material"))) JsonRFn::parseArray(object["material"], material);
		
		if (MapFn::has(object, FixedString("mesh"))) JsonRFn::parseArray(object["mesh"], mesh);
		if (MapFn::has(object, FixedString("unit"))) JsonRFn::parseArray(object["unit"], unit);
		if (MapFn::has(object, FixedString("sprite"))) JsonRFn::parseArray(object["sprite"], sprite);
		if (MapFn::has(object, FixedString("spriteAnimation"))) JsonRFn::parseArray(object["spriteAnimation"], spriteAnimation);
		
		if (MapFn::has(object, FixedString("font"))) JsonRFn::parseArray(object["font"], font);
		if (MapFn::has(object, FixedString("level"))) JsonRFn::parseArray(object["level"], level);
		if (MapFn::has(object, FixedString("physicsConfig"))) JsonRFn::parseArray(object["physicsConfig"], physicsConfig);
		
		if (MapFn::has(object, FixedString("sound"))) JsonRFn::parseArray(object["sound"], sound);

		if (MapFn::has(object, FixedString("script"))) JsonRFn::parseArray(object["script"], script);

		Array<PackageResource::Resource> resources(getDefaultAllocator());

		compileResources("texture", texture, resources, compileOptions);
		compileResources("shader", shader, resources, compileOptions);
		compileResources("mesh", mesh, resources, compileOptions);
		compileResources("material", material, resources, compileOptions);
		compileResources("font", font, resources, compileOptions);
		compileResources("sprite", sprite, resources, compileOptions);
		compileResources("spriteAnimation", spriteAnimation, resources, compileOptions);

		compileResources("unit", unit, resources, compileOptions);
		compileResources("level", level, resources, compileOptions);

		compileResources("sound", sound, resources, compileOptions);
		
		compileResources("script", script, resources, compileOptions);
		
		compileResources("physicsConfig", physicsConfig, resources, compileOptions);
		
		// Write
		compileOptions.write(RESOURCE_VERSION_PACKAGE);
		compileOptions.write(ArrayFn::getCount(resources));

		for (uint32_t i = 0; i < ArrayFn::getCount(resources); ++i)
		{
			compileOptions.write(resources[i].type);
			compileOptions.write(resources[i].name);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_PACKAGE, "Wrong version");

		uint32_t resourceListCount;
		binaryReader.read(resourceListCount);

		PackageResource* packageResource = RIO_NEW(a, PackageResource)(a);
		ArrayFn::resize(packageResource->resources, resourceListCount);
		binaryReader.read(ArrayFn::begin(packageResource->resources), sizeof(PackageResource::Resource)*resourceListCount);

		return packageResource;
	}

	void unload(Allocator& a, void* resource)
	{
		RIO_DELETE(a, (PackageResource*)resource);
	}
} // namespace PackageResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka