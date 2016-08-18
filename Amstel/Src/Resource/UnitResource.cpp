// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/UnitResource.h"
#include "Core/Memory/Allocator.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Resource/ResourceTypes.h"
#include "Resource/UnitCompiler.h"

namespace Rio
{

namespace UnitResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer unitData(getDefaultAllocator());

		UnitCompiler unitCompiler(compileOptions);
		unitCompiler.compileUnit(path);

		compileOptions.write(unitCompiler.getBlob());
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.getSize();
		void* resource = a.allocate(size);
		file.read(resource, size);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_UNIT, "Wrong version");
		return resource;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace UnitResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka