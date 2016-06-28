// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/Containers/Array.h"

#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

struct VertexData
{
	uint32_t verticesCount;
	uint32_t stride;
	char* data;
};

struct IndexData
{
	uint32_t indicesCount;
	char* data; // size = indicesCount * sizeof(uint16_t)
};

struct MeshGeometry
{
	bgfx::VertexDecl vertexDecl;
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
	Obb obb;
	VertexData vertices;
	IndexData indices;
};

struct MeshResource
{
	MeshResource(Allocator& a)
		: geometryNameList(a)
		, meshGeometryList(a)
	{
	}

	const MeshGeometry* getMeshGeometry(StringId32 name) const
	{
		for (uint32_t i = 0; i < ArrayFn::getCount(geometryNameList); ++i)
		{
			if (geometryNameList[i] == name)
			{
				return meshGeometryList[i];
			}
		}

		RIO_ASSERT(false, "Mesh name not found");
		return nullptr;
	}

	Array<StringId32> geometryNameList;
	Array<MeshGeometry*> meshGeometryList;
};

namespace MeshResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka