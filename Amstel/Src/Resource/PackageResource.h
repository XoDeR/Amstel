// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Resource/CompilerTypes.h"
#include "Resource/ResourceTypes.h"

namespace Rio
{

struct PackageResource
{
	struct Resource
	{
		Resource(StringId64 t, StringId64 n)
			: type(t)
			, name(n)
		{
		}

		bool operator<(const Resource& r) const
		{
			return type < r.type;
		}

		bool operator==(const Resource& r) const
		{
			return type == r.type && name == r.name;
		}

		StringId64 type;
		StringId64 name;
	};

	PackageResource(Allocator& a)
		: resources(a)
	{
	}

	Array<Resource> resources;
};

namespace PackageResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace PackageResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka