// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/PackageResource.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Strings/StringId.h"
#include "Core/Memory/TempAllocator.h"

#include "Resource/CompileOptions.h"

namespace Rio
{

namespace PackageResourceInternalFn
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

		if (JsonObjectFn::has(object, "texture")) JsonRFn::parseArray(object["texture"], texture);
		if (JsonObjectFn::has(object, "shader")) JsonRFn::parseArray(object["shader"], shader);
		if (JsonObjectFn::has(object, "material")) JsonRFn::parseArray(object["material"], material);
		
		if (JsonObjectFn::has(object, "mesh")) JsonRFn::parseArray(object["mesh"], mesh);
		if (JsonObjectFn::has(object, "unit")) JsonRFn::parseArray(object["unit"], unit);
		if (JsonObjectFn::has(object, "sprite")) JsonRFn::parseArray(object["sprite"], sprite);
		if (JsonObjectFn::has(object, "spriteAnimation")) JsonRFn::parseArray(object["spriteAnimation"], spriteAnimation);
		
		if (JsonObjectFn::has(object, "font")) JsonRFn::parseArray(object["font"], font);
		if (JsonObjectFn::has(object, "level")) JsonRFn::parseArray(object["level"], level);
		if (JsonObjectFn::has(object, "physicsConfig")) JsonRFn::parseArray(object["physicsConfig"], physicsConfig);
		
		if (JsonObjectFn::has(object, "sound")) JsonRFn::parseArray(object["sound"], sound);

		if (JsonObjectFn::has(object, "script")) JsonRFn::parseArray(object["script"], script);

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
} // namespace PackageResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka