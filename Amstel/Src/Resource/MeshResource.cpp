// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/MeshResource.h"

#include "Core/Math/Aabb.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Containers/Map.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#include "Device/Log.h"

namespace Rio
{

namespace MeshResourceInternalFn
{
	struct MeshCompiler
	{
		MeshCompiler(CompileOptions& compileOptions)
			: compileOptions(compileOptions)
			, positionList(getDefaultAllocator())
			, normalList(getDefaultAllocator())
			, uvList(getDefaultAllocator())
			, tangentList(getDefaultAllocator())
			, binormalList(getDefaultAllocator())
			, positionIndexList(getDefaultAllocator())
			, normalIndexList(getDefaultAllocator())
			, uvIndexList(getDefaultAllocator())
			, tangentIndexList(getDefaultAllocator())
			, binormalIndexList(getDefaultAllocator())
			, vertexBuffer(getDefaultAllocator())
			, indexBuffer(getDefaultAllocator())
		{
		}

		void reset()
		{
			ArrayFn::clear(positionList);
			ArrayFn::clear(normalList);
			ArrayFn::clear(uvList);
			ArrayFn::clear(tangentList);
			ArrayFn::clear(binormalList);

			ArrayFn::clear(positionIndexList);
			ArrayFn::clear(normalIndexList);
			ArrayFn::clear(uvIndexList);
			ArrayFn::clear(tangentIndexList);
			ArrayFn::clear(binormalIndexList);

			vertexStride = 0;
			ArrayFn::clear(vertexBuffer);
			ArrayFn::clear(indexBuffer);

			AabbFn::reset(aabb);

			hasNormal = false;
			hasUv = false;
		}

		void parse(const char* geometry, const char* node)
		{
			TempAllocator4096 ta;
			JsonObject jsonObject(ta);
			JsonObject nodeJsonObject(ta);
			JsonRFn::parse(geometry, jsonObject);
			JsonRFn::parse(node, nodeJsonObject);

			hasNormal = JsonObjectFn::has(jsonObject, "normal");
			hasUv = JsonObjectFn::has(jsonObject, "texCoord");

			parseFloatArray(jsonObject["position"], positionList);

			if (hasNormal == true)
			{
				parseFloatArray(jsonObject["normal"], normalList);
			}
			if (hasUv == true)
			{
				parseFloatArray(jsonObject["texCoord"], uvList);
			}

			parseIndices(jsonObject["indices"]);

			localMatrix = JsonRFn::parseMatrix4x4(nodeJsonObject["localMatrix"]);
		}

		void parseFloatArray(const char* jsonArrayStr, Array<float>& output)
		{
			TempAllocator4096 ta;
			JsonArray jsonArray(ta);
			JsonRFn::parseArray(jsonArrayStr, jsonArray);

			ArrayFn::resize(output, ArrayFn::getCount(jsonArray));
			for (uint32_t i = 0; i < ArrayFn::getCount(jsonArray); ++i)
			{
				output[i] = JsonRFn::parseFloat(jsonArray[i]);
			}
		}

		void parseIndexArray(const char* jsonArrayStr, Array<uint16_t>& output)
		{
			TempAllocator4096 ta;
			JsonArray jsonArray(ta);
			JsonRFn::parseArray(jsonArrayStr, jsonArray);

			ArrayFn::resize(output, ArrayFn::getCount(jsonArray));
			for (uint32_t i = 0; i < ArrayFn::getCount(jsonArray); ++i)
			{
				output[i] = (uint16_t)JsonRFn::parseInt(jsonArray[i]);
			}
		}

		void parseIndices(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject jsonObject(ta);
			JsonRFn::parse(json, jsonObject);

			JsonArray dataJsonArray(ta);
			JsonRFn::parseArray(jsonObject["data"], dataJsonArray);

			parseIndexArray(dataJsonArray[0], positionIndexList);

			if (hasNormal == true)
			{
				parseIndexArray(dataJsonArray[1], normalIndexList);
			}
			if (hasUv == true)
			{
				parseIndexArray(dataJsonArray[2], uvIndexList);
			}
		}

		void compile()
		{
			vertexStride = 0;
			vertexStride += 3 * sizeof(float);
			vertexStride += (hasNormal ? 3 * sizeof(float) : 0);
			vertexStride += (hasUv ? 2 * sizeof(float) : 0);

			// Generate vertex buffer / index buffer
			ArrayFn::resize(indexBuffer, ArrayFn::getCount(positionIndexList));

			uint16_t index = 0;
			for (uint32_t i = 0; i < ArrayFn::getCount(positionIndexList); ++i)
			{
				indexBuffer[i] = index++;

				const uint16_t positionIndex = positionIndexList[i] * 3;
				Vector3 xyz;
				xyz.x = positionList[positionIndex + 0];
				xyz.y = positionList[positionIndex + 1];
				xyz.z = positionList[positionIndex + 2];
				xyz = xyz * localMatrix;
				ArrayFn::push(vertexBuffer, (char*)&xyz, sizeof(xyz));

				if (hasNormal == true)
				{
					const uint16_t normalIndex = normalIndexList[i] * 3;
					Vector3 n;
					n.x = normalList[normalIndex + 0];
					n.y = normalList[normalIndex + 1];
					n.z = normalList[normalIndex + 2];
					ArrayFn::push(vertexBuffer, (char*)&n, sizeof(n));
				}
				if (hasUv == true)
				{
					const uint16_t uvIndex = uvIndexList[i] * 2;
					Vector2 uv;
					uv.x = uvList[uvIndex + 0];
					uv.y = uvList[uvIndex + 1];
					ArrayFn::push(vertexBuffer, (char*)&uv, sizeof(uv));
				}
			}

			// Vertex decl
			vertexDecl.begin();
			vertexDecl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

			if (hasNormal == true)
			{
				vertexDecl.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true);
			}
			if (hasUv == true)
			{
				vertexDecl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			}

			vertexDecl.end();

			// Bounds
			AabbFn::reset(aabb);
			AabbFn::addPoints(aabb
				, ArrayFn::getCount(positionList) / 3
				, sizeof(float) * 3
				, ArrayFn::begin(positionList)
				);
			aabb = AabbFn::getTransformed(aabb, localMatrix);

			obb.transformMatrix = createMatrix4x4(QUATERNION_IDENTITY, AabbFn::getCenter(aabb));
			obb.halfExtents.x = (aabb.max.x - aabb.min.x) * 0.5f;
			obb.halfExtents.y = (aabb.max.y - aabb.min.y) * 0.5f;
			obb.halfExtents.z = (aabb.max.z - aabb.min.z) * 0.5f;
		}

		void write()
		{
			compileOptions.write(vertexDecl);
			compileOptions.write(obb);

			compileOptions.write(ArrayFn::getCount(vertexBuffer) / vertexStride);
			compileOptions.write(vertexStride);
			compileOptions.write(ArrayFn::getCount(indexBuffer));

			compileOptions.write(vertexBuffer);
			compileOptions.write(ArrayFn::begin(indexBuffer), ArrayFn::getCount(indexBuffer) * sizeof(uint16_t));
		}

		CompileOptions& compileOptions;

		Array<float> positionList;
		Array<float> normalList;
		Array<float> uvList;
		Array<float> tangentList;
		Array<float> binormalList;

		Array<uint16_t> positionIndexList;
		Array<uint16_t> normalIndexList;
		Array<uint16_t> uvIndexList;
		Array<uint16_t> tangentIndexList;
		Array<uint16_t> binormalIndexList;

		Matrix4x4 localMatrix = MATRIX4X4_IDENTITY;

		uint32_t vertexStride = 0;
		Array<char> vertexBuffer;
		Array<uint16_t> indexBuffer;

		Aabb aabb;
		Obb obb;

		bgfx::VertexDecl vertexDecl;

		bool hasNormal = false;
		bool hasUv = false;
	};

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		JsonObject geometries(ta);
		JsonRFn::parse(jsonObject["geometries"], geometries);
		JsonObject nodes(ta);
		JsonRFn::parse(jsonObject["nodes"], nodes);

		compileOptions.write(RESOURCE_VERSION_MESH);
		compileOptions.write(JsonObjectFn::getCount(geometries));

		MeshCompiler meshCompiler(compileOptions);

		auto begin = JsonObjectFn::begin(geometries);
		auto end = JsonObjectFn::end(geometries);
		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* geometry = begin->pair.second;
			const char* node = nodes[key];

			const StringId32 name(key.getData(), key.getLength());
			compileOptions.write(name.id);

			meshCompiler.reset();
			meshCompiler.parse(geometry, node);
			meshCompiler.compile();
			meshCompiler.write();
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_MESH, "Wrong version");

		uint32_t geometryListCount;
		binaryReader.read(geometryListCount);

		MeshResource* meshResource = RIO_NEW(a, MeshResource)(a);
		ArrayFn::resize(meshResource->geometryNameList, geometryListCount);
		ArrayFn::resize(meshResource->meshGeometryList, geometryListCount);

		for (uint32_t i = 0; i < geometryListCount; ++i)
		{
			StringId32 name;
			binaryReader.read(name);

			bgfx::VertexDecl vertexDecl;
			binaryReader.read(vertexDecl);

			Obb obb;
			binaryReader.read(obb);

			uint32_t verticesCount;
			binaryReader.read(verticesCount);

			uint32_t stride;
			binaryReader.read(stride);

			uint32_t indicesCount;
			binaryReader.read(indicesCount);

			const uint32_t verticesSize = verticesCount * stride;
			const uint32_t indicesSize = indicesCount * sizeof(uint16_t);

			const uint32_t size = sizeof(MeshGeometry) + verticesSize + indicesSize;

			MeshGeometry* meshGeometry = (MeshGeometry*)a.allocate(size);
			meshGeometry->obb = obb;
			meshGeometry->vertexDecl = vertexDecl;
			meshGeometry->vertexBufferHandle = BGFX_INVALID_HANDLE;
			meshGeometry->indexBufferHandle = BGFX_INVALID_HANDLE;
			meshGeometry->vertices.verticesCount = verticesCount;
			meshGeometry->vertices.stride = stride;
			meshGeometry->vertices.data = (char*)&meshGeometry[1];
			meshGeometry->indices.indicesCount = indicesCount;
			meshGeometry->indices.data = meshGeometry->vertices.data + verticesSize;

			binaryReader.read(meshGeometry->vertices.data, verticesSize);
			binaryReader.read(meshGeometry->indices.data, indicesSize);

			meshResource->geometryNameList[i] = name;
			meshResource->meshGeometryList[i] = meshGeometry;
		}

		return meshResource;
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		MeshResource* meshResource = (MeshResource*)resourceManager.get(RESOURCE_TYPE_MESH, id);

		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->meshGeometryList); ++i)
		{
			MeshGeometry& meshGeometry = *meshResource->meshGeometryList[i];

			const uint32_t verticesSize = meshGeometry.vertices.verticesCount * meshGeometry.vertices.stride;
			const uint32_t indicesSize = meshGeometry.indices.indicesCount * sizeof(uint16_t);

			const bgfx::Memory* vertexMemory = bgfx::makeRef(meshGeometry.vertices.data, verticesSize);
			const bgfx::Memory* indexMemory = bgfx::makeRef(meshGeometry.indices.data, indicesSize);

			bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(vertexMemory, meshGeometry.vertexDecl);
			bgfx::IndexBufferHandle indexBufferHandle  = bgfx::createIndexBuffer(indexMemory);
			RIO_ASSERT(bgfx::isValid(vertexBufferHandle), "Invalid vertex buffer");
			RIO_ASSERT(bgfx::isValid(indexBufferHandle), "Invalid index buffer");

			meshGeometry.vertexBufferHandle = vertexBufferHandle;
			meshGeometry.indexBufferHandle = indexBufferHandle;
		}
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		MeshResource* meshResource = (MeshResource*)resourceManager.get(RESOURCE_TYPE_MESH, id);

		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->meshGeometryList); ++i)
		{
			MeshGeometry& meshGeometry = *meshResource->meshGeometryList[i];
			bgfx::destroyVertexBuffer(meshGeometry.vertexBufferHandle);
			bgfx::destroyIndexBuffer(meshGeometry.indexBufferHandle);
		}
	}

	void unload(Allocator& a, void* resource)
	{
		MeshResource* meshResource = (MeshResource*)resource;
		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->meshGeometryList); ++i)
		{
			a.deallocate(meshResource->meshGeometryList[i]);
		}
		RIO_DELETE(a, (MeshResource*)resource);
	}
} // namespace MeshResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka