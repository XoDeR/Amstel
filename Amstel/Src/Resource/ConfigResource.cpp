// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ConfigResource.h"
#include "Core/Memory/Allocator.h"
#include "Core/Containers/Map.h"
#include "Core/Json/JsonR.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

namespace ConfigResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator1024 ta;
		JsonObject boot(ta);
		JsonRFn::parse(buffer, boot);

		const char* bootScriptJson  = MapFn::get(boot, FixedString("bootScript"), (const char*)nullptr);
		const char* bootPackageJson = MapFn::get(boot, FixedString("bootPackage"), (const char*)nullptr);
		RESOURCE_COMPILER_ASSERT(bootScriptJson != nullptr, compileOptions, "'bootScript' must be specified.");
		RESOURCE_COMPILER_ASSERT(bootPackageJson != nullptr, compileOptions, "'bootPackage' must be specified.");

		DynamicString bootScript(ta);
		DynamicString bootPackage(ta);
		JsonRFn::parseString(bootScriptJson, bootScript);
		JsonRFn::parseString(bootPackageJson, bootPackage);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_SCRIPT, bootScript.getCStr(), compileOptions);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_PACKAGE, bootPackage.getCStr(), compileOptions);

		compileOptions.write(buffer);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.getSize();
		char* result = (char*)a.allocate(size + 1);
		file.read(result, size);
		result[size] = '\0';
		return result;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}
} // namespace ConfigResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka