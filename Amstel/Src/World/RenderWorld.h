// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/ResourceTypes.h"
#include "Resource/MeshResource.h"
#include "World/WorldTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

class RenderWorld
{
public:
	RenderWorld(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager);
	~RenderWorld();
	MeshInstance createMesh(UnitId id, const MeshRendererDesc& meshRendererDesc, const Matrix4x4& transform);
	void destroyMesh(MeshInstance i);
	void getMeshInstanceList(UnitId id, Array<MeshInstance>& instances);
	void setMeshMaterial(MeshInstance i, StringId64 id);
	void setMeshVisible(MeshInstance i, bool visible);
	Obb getMeshObb(MeshInstance i);
	float getMeshRaycast(MeshInstance i, const Vector3& from, const Vector3& direction);

	SpriteInstance createSprite(UnitId id, const SpriteRendererDesc& spriteRendererDesc, const Matrix4x4& transform);
	void destroySprite(SpriteInstance i);
	// Returns the sprite instances of the unit <id>
	void getSpriteInstanceList(UnitId id, Array<SpriteInstance>& instances);
	void setSpriteMaterial(SpriteInstance i, StringId64 id);
	void setSpriteFrame(SpriteInstance i, uint32_t index);
	void setSpriteVisible(SpriteInstance i, bool visible);

	LightInstance createLight(UnitId id, const LightDesc& lightDesc, const Matrix4x4& transform);
	void destroyLight(LightInstance i);
	// Returns the light of the unit <id>
	LightInstance getLight(UnitId id);
	// Returns the type of the light <i>
	LightType::Enum getLightType(LightInstance i);
	// Returns the color of the light <i>
	Color4 getLightColor(LightInstance i);
	// Returns the range of the light <i>
	float getLightRange(LightInstance i);
	// Returns the intensity of the light <i>
	float getLightIntensity(LightInstance i);
	// Returns the spot angle of the light <i>
	float getLightSpotAngle(LightInstance i);
	// Sets the <type> of the light <i>
	void setLightType(LightInstance i, LightType::Enum type);
	// Sets the <color> of the light <i>
	void setLightColor(LightInstance i, const Color4& color);
	// Sets the <range> of the light <i>
	void setLightRange(LightInstance i, float range);
	// Sets the <intensity> of the light <i>
	void setLightIntensity(LightInstance i, float intensity);
	// Sets the spot <angle> of the light <i>
	void setLightSpotAngle(LightInstance i, float angle);

	void updateTransforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world);

	void render(const Matrix4x4& view, const Matrix4x4& projection);
	void enableDebugDrawing(bool enable);
	void drawDebug(DebugLine& debugLine);

	static const uint32_t MARKER = 0xc82277de;
private:

	static void unitDestroyedCallback(UnitId id, void* userPtr)
	{
		((RenderWorld*)userPtr)->unitDestroyedCallback(id);
	}

	void unitDestroyedCallback(UnitId id);

	struct MeshManager
	{
		struct MeshData
		{
			bgfx::VertexBufferHandle vertexBufferHandle;
			bgfx::IndexBufferHandle indexBufferHandle;
		};

		struct MeshInstanceData
		{
			uint32_t size;
			uint32_t capacity;
			void* buffer;

			uint32_t firstHidden;

			UnitId* unit;
			const MeshResource** resource;
			const MeshGeometry** geometry;
			MeshData* mesh;
			StringId64* material;
			Matrix4x4* world;
			Obb* obb;
			MeshInstance* nextInstance;
		};

		MeshManager(Allocator& a)
			: allocator(&a)
			, unitIdToMeshMap(a)
		{
			memset(&data, 0, sizeof(data));
		}

		void allocate(uint32_t meshInstancesCount);
		void grow();
		MeshInstance create(UnitId id, const MeshResource* meshResource, const MeshGeometry* meshGeometry, StringId64 material, const Matrix4x4& transform);
		void destroy(MeshInstance i);
		bool has(UnitId id);
		MeshInstance getFirst(UnitId id);
		MeshInstance getNext(MeshInstance i);
		MeshInstance getPrev(MeshInstance i);
		void addNode(MeshInstance first, MeshInstance i);
		void removeNode(MeshInstance first, MeshInstance i);
		void swapNode(MeshInstance a, MeshInstance b);
		void destroy();

		MeshInstance makeInstance(uint32_t i) 
		{ 
			MeshInstance meshInstance = { i };
			return meshInstance;
		}
		bool getIsValid(MeshInstance i) 
		{ 
			return i.i != UINT32_MAX; 
		}

		Allocator* allocator;
		HashMap<UnitId, uint32_t> unitIdToMeshMap;
		MeshInstanceData data;
	};

	struct SpriteManager
	{
		struct SpriteData
		{
			bgfx::VertexBufferHandle vertexBufferHandle;
			bgfx::IndexBufferHandle indexBufferHandle;
		};

		struct SpriteInstanceData
		{
			uint32_t size;
			uint32_t capacity;
			void* buffer;

			uint32_t firstHidden;

			UnitId* unit;
			const SpriteResource** resource;
			SpriteData* sprite;
			StringId64* material;
			uint32_t* frame;
			Matrix4x4* world;
			Aabb* aabb;
			SpriteInstance* nextInstance;
		};

		SpriteManager(Allocator& a)
			: allocator(&a)
			, unitIdToSpriteMap(a)
		{
			memset(&data, 0, sizeof(data));
		}

		void allocate(uint32_t spriteInstancesCount);
		void grow();
		SpriteInstance create(UnitId id, const SpriteResource* spriteResource, StringId64 material, const Matrix4x4& transform);
		void destroy(SpriteInstance i);
		bool has(UnitId id);
		SpriteInstance getFirst(UnitId id);
		SpriteInstance getNext(SpriteInstance i);
		SpriteInstance getPrev(SpriteInstance i);
		void addNode(SpriteInstance first, SpriteInstance i);
		void removeNode(SpriteInstance first, SpriteInstance i);
		void swapNode(SpriteInstance a, SpriteInstance b);
		void destroy();

		SpriteInstance makeInstance(uint32_t i) 
		{ 
			SpriteInstance spriteInstance = { i };
			return spriteInstance;
		}
		bool getIsValid(SpriteInstance i)
		{ 
			return i.i != UINT32_MAX; 
		}

		Allocator* allocator;
		HashMap<UnitId, uint32_t> unitIdToSpriteMap;
		SpriteInstanceData data;
	};

	struct LightManager
	{
		struct LightInstanceData
		{
			uint32_t size;
			uint32_t capacity;
			void* buffer;

			UnitId* unit;
			Matrix4x4* world;
			float* range;
			float* intensity;
			float* spotAngle;
			Color4* color;
			uint32_t* type; // LightType::Enum
		};

		LightManager(Allocator& a)
			: allocator(&a)
			, unitIdToLightMap(a)
		{
			memset(&data, 0, sizeof(data));
		}

		LightInstance create(UnitId id, const LightDesc& lightDesc, const Matrix4x4& transform);
		void destroy(LightInstance i);
		bool has(UnitId id);
		LightInstance getLight(UnitId id);
		void allocate(uint32_t lightInstancesCount);
		void grow();
		void destroy();

		LightInstance makeInstance(uint32_t i) 
		{ 
			LightInstance lightInstance = { i };
			return lightInstance;
		}
		bool getIsValid(LightInstance i)
		{
			return i.i != UINT32_MAX; 
		}

		Allocator* allocator;
		HashMap<UnitId, uint32_t> unitIdToLightMap;
		LightInstanceData data;
	};

	uint32_t marker;

	Allocator* allocator;
	ResourceManager* resourceManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	UnitManager* unitManager;

	bgfx::UniformHandle uniformLightPosition;
	bgfx::UniformHandle uniformLightDirection;
	bgfx::UniformHandle uniformLightColor;

	bool isDebugDrawing = false;
	MeshManager meshManager;
	SpriteManager spriteManager;
	LightManager lightManager;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka