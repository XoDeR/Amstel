// Copyright (c) 2016 Volodymyr Syvochka
#include "World/RenderWorld.h"

#include "Core/Math/Aabb.h"
#include "Core/Math/Color4.h"
#include "Core/Containers/HashMap.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/Matrix4x4.h"

#include "Resource/MeshResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/SpriteResource.h"

#include "World/DebugLine.h"
#include "World/Material.h"
#include "World/MaterialManager.h"
#include "World/UnitManager.h"

#include <bgfx/bgfx.h>

namespace Rio
{

RenderWorld::RenderWorld(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager)
	: marker(RENDER_WORLD_MARKER)
	, allocator(&a)
	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, unitManager(&unitManager)
	, meshManager(a)
	, spriteManager(a)
	, lightManager(a)
{
	unitManager.registerDestroyFunction(RenderWorld::unitDestroyedCallback, this);

	uniformLightPosition = bgfx::createUniform("uniformLightPosition", bgfx::UniformType::Vec4);
	uniformLightDirection = bgfx::createUniform("uniformLightDirection", bgfx::UniformType::Vec4);
	uniformLightColor = bgfx::createUniform("uniformLightColor", bgfx::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	unitManager->unregisterDestroyFunction(this);

	bgfx::destroyUniform(uniformLightPosition);
	bgfx::destroyUniform(uniformLightDirection);
	bgfx::destroyUniform(uniformLightColor);

	meshManager.destroy();
	spriteManager.destroy();
	lightManager.destroy();

	marker = 0;
}

MeshInstance RenderWorld::meshCreate(UnitId id, const MeshRendererDesc& meshRendererDesc, const Matrix4x4& transform)
{
	const MeshResource* meshResource = (const MeshResource*)resourceManager->get(RESOURCE_TYPE_MESH, meshRendererDesc.meshResource);
	const MeshGeometry* meshGeometry = meshResource->getMeshGeometry(meshRendererDesc.geometryName);
	materialManager->createMaterial(meshRendererDesc.materialResource);

	return meshManager.create(id, meshResource, meshGeometry, meshRendererDesc.materialResource, transform);
}

void RenderWorld::meshDestroy(MeshInstance i)
{
	meshManager.destroy(i);
}

void RenderWorld::meshGetInstanceList(UnitId id, Array<MeshInstance>& meshInstanceList)
{
	MeshInstance meshInstance = meshManager.getFirst(id);

	while (meshManager.getIsValid(meshInstance))
	{
		ArrayFn::pushBack(meshInstanceList, meshInstance);
		meshInstance = meshManager.getNext(meshInstance);
	}
}

void RenderWorld::meshSetMaterial(MeshInstance i, StringId64 id)
{
	RIO_ASSERT(i.i < meshManager.data.size, "Index out of bounds");
	meshManager.data.material[i.i] = id;
}

void RenderWorld::meshSetVisible(MeshInstance i, bool visible)
{
	RIO_ASSERT(i.i < meshManager.data.size, "Index out of bounds");
}

Obb RenderWorld::meshGetObb(MeshInstance i)
{
	RIO_ASSERT(i.i < meshManager.data.size, "Index out of bounds");
	const Matrix4x4& world = meshManager.data.world[i.i];
	const Obb& obb = meshManager.data.obb[i.i];

	Obb result;
	result.transformMatrix = obb.transformMatrix * world;
	result.halfExtents = obb.halfExtents;

	return result;
}

float RenderWorld::meshGetRaycast(MeshInstance i, const Vector3& from, const Vector3& direction)
{
	RIO_ASSERT(i.i < meshManager.data.size, "Index out of bounds");
	const MeshGeometry* meshGeometry = meshManager.data.geometry[i.i];
	return getRayMeshIntersection(from
		, direction
		, meshManager.data.world[i.i]
		, meshGeometry->vertices.data
		, meshGeometry->vertices.stride
		, (uint16_t*)meshGeometry->indices.data
		, meshGeometry->indices.indicesCount
		);
}

SpriteInstance RenderWorld::spriteCreate(UnitId id, const SpriteRendererDesc& spriteRendererDesc, const Matrix4x4& transform)
{
	const SpriteResource* spriteResource = (const SpriteResource*)resourceManager->get(RESOURCE_TYPE_SPRITE, spriteRendererDesc.spriteResourceName);
	materialManager->createMaterial(spriteRendererDesc.materialResource);

	return spriteManager.create(id, spriteResource, spriteRendererDesc.materialResource, transform);
}

void RenderWorld::spriteDestroy(SpriteInstance i)
{
	RIO_ASSERT(i.i < spriteManager.data.size, "Index out of bounds");
	spriteManager.destroy(i);
}

SpriteInstance RenderWorld::spriteGet(UnitId unitId)
{
	return spriteManager.getSprite(unitId);
}

void RenderWorld::spriteSetMaterial(SpriteInstance i, StringId64 id)
{
	RIO_ASSERT(i.i < spriteManager.data.size, "Index out of bounds");
	spriteManager.data.material[i.i] = id;
}

void RenderWorld::spriteSetVisible(SpriteInstance i, bool visible)
{
	RIO_ASSERT(i.i < spriteManager.data.size, "Index out of bounds");
}

void RenderWorld::spriteSetFrame(SpriteInstance i, uint32_t index)
{
	RIO_ASSERT(i.i < spriteManager.data.size, "Index out of bounds");
	spriteManager.data.frame[i.i] = index;
}

LightInstance RenderWorld::lightCreate(UnitId id, const LightDesc& lightDesc, const Matrix4x4& transform)
{
	return lightManager.create(id, lightDesc, transform);
}

void RenderWorld::lightDestroy(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.destroy(i);
}

LightInstance RenderWorld::lightGet(UnitId id)
{
	return lightManager.getLight(id);
}

Color4 RenderWorld::lightGetColor(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	return lightManager.data.color[i.i];
}

LightType::Enum RenderWorld::lightGetType(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	return (LightType::Enum)lightManager.data.type[i.i];
}

float RenderWorld::lightGetRange(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	return lightManager.data.range[i.i];
}

float RenderWorld::lightGetIntensity(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	return lightManager.data.intensity[i.i];
}

float RenderWorld::lightGetSpotAngle(LightInstance i)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	return lightManager.data.spotAngle[i.i];
}

void RenderWorld::lightSetColor(LightInstance i, const Color4& color)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.data.color[i.i] = color;
}

void RenderWorld::lightSetType(LightInstance i, LightType::Enum type)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.data.type[i.i] = type;
}

void RenderWorld::lightSetRange(LightInstance i, float range)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.data.range[i.i] = range;
}

void RenderWorld::lightSetIntensity(LightInstance i, float intensity)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.data.intensity[i.i] = intensity;
}

void RenderWorld::lightSetSpotAngle(LightInstance i, float angle)
{
	RIO_ASSERT(i.i < lightManager.data.size, "Index out of bounds");
	lightManager.data.spotAngle[i.i] = angle;
}

void RenderWorld::updateTransforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world)
{
	MeshManager::MeshInstanceData& meshInstanceData = meshManager.data;
	SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.data;
	LightManager::LightInstanceData& lightInstanceData = lightManager.data;

	for (; begin != end; ++begin, ++world)
	{
		if (meshManager.has(*begin) == true)
		{
			MeshInstance meshInstance = meshManager.getFirst(*begin);
			meshInstanceData.world[meshInstance.i] = *world;
		}

		if (spriteManager.has(*begin) == true)
		{
			SpriteInstance spriteInstance = spriteManager.getSprite(*begin);
			spriteInstanceData.world[spriteInstance.i] = *world;
		}

		if (lightManager.has(*begin) == true)
		{
			LightInstance lightInstance = lightManager.getLight(*begin);
			lightInstanceData.world[lightInstance.i] = *world;
		}
	}
}

void RenderWorld::render(const Matrix4x4& view, const Matrix4x4& projection)
{
	MeshManager::MeshInstanceData& meshInstanceData = meshManager.data;
	SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.data;
	LightManager::LightInstanceData& lightInstanceData = lightManager.data;

	for (uint32_t lightInstanceIndex = 0; lightInstanceIndex < lightInstanceData.size; ++lightInstanceIndex)
	{
		const Vector4 ligthDirection = normalize(lightInstanceData.world[lightInstanceIndex].z) * view;
		const Vector3 ligthPosition = getTranslation(lightInstanceData.world[lightInstanceIndex]);

		bgfx::setUniform(uniformLightPosition, getFloatPointer(ligthPosition));
		bgfx::setUniform(uniformLightDirection, getFloatPointer(ligthDirection));
		bgfx::setUniform(uniformLightColor, getFloatPointer(lightInstanceData.color[lightInstanceIndex]));

		// Render meshes
		for (uint32_t i = 0; i < meshInstanceData.firstHidden; ++i)
		{
			bgfx::setTransform(getFloatPointer(meshInstanceData.world[i]));
			bgfx::setVertexBuffer(meshInstanceData.mesh[i].vertexBufferHandle);
			bgfx::setIndexBuffer(meshInstanceData.mesh[i].indexBufferHandle);

			materialManager->get(meshInstanceData.material[i])->bind(*resourceManager, *shaderManager);
		}
	}

	// Render sprites
	for (uint32_t i = 0; i < spriteInstanceData.firstHidden; ++i)
	{
		bgfx::setVertexBuffer(spriteInstanceData.sprite[i].vertexBufferHandle);
		bgfx::setIndexBuffer(spriteInstanceData.sprite[i].indexBufferHandle, spriteInstanceData.frame[i] * 6, 6);
		bgfx::setTransform(getFloatPointer(spriteInstanceData.world[i]));

		materialManager->get(spriteInstanceData.material[i])->bind(*resourceManager, *shaderManager);
	}
}

void RenderWorld::lightDebugDraw(LightInstance i, DebugLine& debugLine)
{
	LightManager::LightInstanceData& lightInstanceData = lightManager.data;

	const Vector3 position = getTranslation(lightInstanceData.world[i.i]);
	const Vector3 direction = -getAxisZ(lightInstanceData.world[i.i]);

	switch (lightInstanceData.type[i.i])
	{
	case LightType::DIRECTIONAL:
	{
		const Vector3 end = position + direction * 3.0f;
		debugLine.addLine(position, end, COLOR4_YELLOW);
		debugLine.addCone(position + direction * 2.8f, end, 0.1f, COLOR4_YELLOW);
	}
	break;
	case LightType::OMNI:
	{
		debugLine.addSphere(position, lightInstanceData.range[i.i], COLOR4_YELLOW);
	}
	break;
	case LightType::SPOT:
	{
		const float angle = lightInstanceData.spotAngle[i.i];
		const float range = lightInstanceData.range[i.i];
		const float radius = tan(angle)*range;
		debugLine.addCone(position + range * direction, position, radius, COLOR4_YELLOW);
	}
	break;
	default:
	{
		RIO_FATAL("Unknown light type");
	}
	break;
	}
}

void RenderWorld::debugDraw(DebugLine& debugLine)
{
	if (!isDebugDrawing)
	{
		return;
	}

	MeshManager::MeshInstanceData& meshInstanceData = meshManager.data;
	SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.data;
	LightManager::LightInstanceData& lightInstanceData = lightManager.data;

	for (uint32_t i = 0; i < meshInstanceData.size; ++i)
	{
		const Obb& obb = meshInstanceData.obb[i];
		const Matrix4x4& world = meshInstanceData.world[i];
		debugLine.addObb(obb.transformMatrix * world, obb.halfExtents, COLOR4_RED);
	}

	for (uint32_t i = 0; i < lightInstanceData.size; ++i)
	{
		lightDebugDraw({ i }, debugLine);
	}
}

void RenderWorld::enableDebugDrawing(bool enable)
{
	isDebugDrawing = enable;
}

void RenderWorld::unitDestroyedCallback(UnitId id)
{
	{
		MeshInstance current = meshManager.getFirst(id);
		MeshInstance next;

		while (meshManager.getIsValid(current) == true)
		{
			next = meshManager.getNext(current);
			meshDestroy(current);
			current = next;
		}
	}

	{
		SpriteInstance firstSprite = spriteGet(id);

		if (spriteManager.getIsValid(firstSprite))
		{
			spriteDestroy(firstSprite);
		}
	}

	{
		LightInstance first = lightGet(id);

		if (lightManager.getIsValid(first))
		{
			lightDestroy(first);
		}
	}
}

void RenderWorld::MeshManager::allocate(uint32_t meshInstancesCount)
{
	RIO_ENSURE(num > this->data.size);

	const uint32_t bytes = 0
		+ meshInstancesCount * sizeof(UnitId) + alignof(UnitId)
		+ meshInstancesCount * sizeof(MeshResource*) + alignof(MeshResource*)
		+ meshInstancesCount * sizeof(MeshGeometry*) + alignof(MeshGeometry*)
		+ meshInstancesCount * sizeof(MeshData) + alignof(MeshData)
		+ meshInstancesCount * sizeof(StringId64) + alignof(StringId64)
		+ meshInstancesCount * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ meshInstancesCount * sizeof(Obb) + alignof(Obb)
		+ meshInstancesCount * sizeof(MeshInstance) + alignof(MeshInstance)
		;

	MeshInstanceData newMeshInstanceData;
	newMeshInstanceData.size = this->data.size;
	newMeshInstanceData.capacity = meshInstancesCount;
	newMeshInstanceData.buffer = allocator->allocate(bytes);
	newMeshInstanceData.firstHidden = this->data.firstHidden;

	newMeshInstanceData.unit = (UnitId*)(newMeshInstanceData.buffer);

	newMeshInstanceData.resource = (const MeshResource**)MemoryFn::alignTop(newMeshInstanceData.unit + meshInstancesCount, alignof(const MeshResource*));
	newMeshInstanceData.geometry = (const MeshGeometry**)MemoryFn::alignTop(newMeshInstanceData.resource + meshInstancesCount, alignof(const MeshGeometry*));
	newMeshInstanceData.mesh = (MeshData*)MemoryFn::alignTop(newMeshInstanceData.geometry + meshInstancesCount, alignof(MeshData));
	newMeshInstanceData.material = (StringId64*)MemoryFn::alignTop(newMeshInstanceData.mesh + meshInstancesCount, alignof(StringId64));
	newMeshInstanceData.world = (Matrix4x4*)MemoryFn::alignTop(newMeshInstanceData.material + meshInstancesCount, alignof(Matrix4x4));
	newMeshInstanceData.obb = (Obb*)MemoryFn::alignTop(newMeshInstanceData.world + meshInstancesCount, alignof(Obb));
	newMeshInstanceData.nextInstance = (MeshInstance*)MemoryFn::alignTop(newMeshInstanceData.obb + meshInstancesCount, alignof(MeshInstance));

	memcpy(newMeshInstanceData.unit, this->data.unit, this->data.size * sizeof(UnitId));
	memcpy(newMeshInstanceData.resource, this->data.resource, this->data.size * sizeof(MeshResource*));
	memcpy(newMeshInstanceData.geometry, this->data.geometry, this->data.size * sizeof(MeshGeometry*));
	memcpy(newMeshInstanceData.mesh, this->data.mesh, this->data.size * sizeof(MeshData));
	memcpy(newMeshInstanceData.material, this->data.material, this->data.size * sizeof(StringId64));
	memcpy(newMeshInstanceData.world, this->data.world, this->data.size * sizeof(Matrix4x4));
	memcpy(newMeshInstanceData.obb, this->data.obb, this->data.size * sizeof(Obb));
	memcpy(newMeshInstanceData.nextInstance, this->data.nextInstance, this->data.size * sizeof(MeshInstance));

	allocator->deallocate(this->data.buffer);
	this->data = newMeshInstanceData;
}

void RenderWorld::MeshManager::grow()
{
	allocate(this->data.capacity * 2 + 1);
}

MeshInstance RenderWorld::MeshManager::create(UnitId id, const MeshResource* meshResource, const MeshGeometry* meshGeometry, StringId64 material, const Matrix4x4& transform)
{
	if (this->data.size == this->data.capacity)
	{
		grow();
	}

	const uint32_t last = this->data.size;

	this->data.unit[last] = id;
	this->data.resource[last] = meshResource;
	this->data.geometry[last] = meshGeometry;
	this->data.mesh[last].vertexBufferHandle = meshGeometry->vertexBufferHandle;
	this->data.mesh[last].indexBufferHandle = meshGeometry->indexBufferHandle;
	this->data.material[last] = material;
	this->data.world[last] = transform;
	this->data.obb[last] = meshGeometry->obb;
	this->data.nextInstance[last] = makeInstance(UINT32_MAX);

	++this->data.size;
	++this->data.firstHidden;

	MeshInstance current = getFirst(id);
	if (getIsValid(current) == false)
	{
		HashMapFn::set(unitIdToMeshMap, id, last);
	}
	else
	{
		addNode(current, makeInstance(last));
	}

	return makeInstance(last);
}

void RenderWorld::MeshManager::destroy(MeshInstance i)
{
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	const uint32_t last = this->data.size - 1;
	const UnitId unitId = this->data.unit[i.i];
	const MeshInstance firstMeshInstance = getFirst(unitId);
	const MeshInstance lastMeshInstance = makeInstance(last);

	swapNode(lastMeshInstance, i);
	removeNode(firstMeshInstance, i);

	this->data.unit[i.i] = this->data.unit[last];
	this->data.resource[i.i] = this->data.resource[last];
	this->data.geometry[i.i] = this->data.geometry[last];
	this->data.mesh[i.i].vertexBufferHandle = this->data.mesh[last].vertexBufferHandle;
	this->data.mesh[i.i].indexBufferHandle = this->data.mesh[last].indexBufferHandle;
	this->data.material[i.i] = this->data.material[last];
	this->data.world[i.i] = this->data.world[last];
	this->data.obb[i.i] = this->data.obb[last];
	this->data.nextInstance[i.i] = this->data.nextInstance[last];

	--this->data.size;
	--this->data.firstHidden;
}

bool RenderWorld::MeshManager::has(UnitId id)
{
	return getIsValid(getFirst(id));
}

MeshInstance RenderWorld::MeshManager::getFirst(UnitId id)
{
	return makeInstance(HashMapFn::get(unitIdToMeshMap, id, UINT32_MAX));
}

MeshInstance RenderWorld::MeshManager::getNext(MeshInstance i)
{
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");
	return this->data.nextInstance[i.i];
}

MeshInstance RenderWorld::MeshManager::getPrev(MeshInstance i)
{
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	const UnitId u = this->data.unit[i.i];

	MeshInstance current = getFirst(u);
	MeshInstance prev = { UINT32_MAX };

	while (current.i != i.i)
	{
		prev = current;
		current = getNext(current);
	}

	return prev;
}

void RenderWorld::MeshManager::addNode(MeshInstance first, MeshInstance i)
{
	RIO_ASSERT(first.i < this->data.size, "Index out of bounds");
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	MeshInstance current = first;
	while (getIsValid(getNext(current)))
	{
		current = getNext(current);
	}

	this->data.nextInstance[current.i] = i;
}

void RenderWorld::MeshManager::removeNode(MeshInstance first, MeshInstance i)
{
	RIO_ASSERT(first.i < this->data.size, "Index out of bounds");
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	const UnitId u = this->data.unit[first.i];

	if (i.i == first.i)
	{
		if (!getIsValid(getNext(i)))
		{
			HashMapFn::set(unitIdToMeshMap, u, UINT32_MAX);
		}
		else
		{
			HashMapFn::set(unitIdToMeshMap, u, getNext(i).i);
		}
	}
	else
	{
		MeshInstance prev = getPrev(i);
		this->data.nextInstance[prev.i] = getNext(i);
	}
}

void RenderWorld::MeshManager::swapNode(MeshInstance a, MeshInstance b)
{
	RIO_ASSERT(a.i < this->data.size, "Index out of bounds");
	RIO_ASSERT(b.i < this->data.size, "Index out of bounds");

	const UnitId unitId = this->data.unit[a.i];
	const MeshInstance firstMeshInstance = getFirst(unitId);

	if (a.i == firstMeshInstance.i)
	{
		HashMapFn::set(unitIdToMeshMap, unitId, b.i);
	}
	else
	{
		const MeshInstance prevA = getPrev(a);
		RIO_ENSURE(prevA.i != a.i);
		this->data.nextInstance[prevA.i] = b;
	}
}

void RenderWorld::MeshManager::destroy()
{
	allocator->deallocate(this->data.buffer);
}

void RenderWorld::SpriteManager::allocate(uint32_t spriteInstancesCount)
{
	RIO_ENSURE(spriteInstancesCount > this->data.size);

	const uint32_t bytes = 0
		+ spriteInstancesCount + sizeof(UnitId) + alignof(UnitId)
		+ spriteInstancesCount + sizeof(SpriteResource**) + alignof(SpriteResource*)
		+ spriteInstancesCount + sizeof(SpriteData) + alignof(SpriteData)
		+ spriteInstancesCount + sizeof(StringId64) + alignof(StringId64)
		+ spriteInstancesCount + sizeof(uint32_t) + alignof(uint32_t)
		+ spriteInstancesCount + sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ spriteInstancesCount + sizeof(Aabb) + alignof(Aabb)
		+ spriteInstancesCount + sizeof(SpriteInstance) + alignof(SpriteInstance)
		;

	SpriteInstanceData newSpriteInstanceData;
	newSpriteInstanceData.size = this->data.size;
	newSpriteInstanceData.capacity = spriteInstancesCount;
	newSpriteInstanceData.buffer = allocator->allocate(bytes);
	newSpriteInstanceData.firstHidden = this->data.firstHidden;

	newSpriteInstanceData.unit = (UnitId*)(newSpriteInstanceData.buffer);
	newSpriteInstanceData.resource = (const SpriteResource**)MemoryFn::alignTop(newSpriteInstanceData.unit + spriteInstancesCount, alignof(const SpriteResource*));
	newSpriteInstanceData.sprite = (SpriteData*)MemoryFn::alignTop(newSpriteInstanceData.resource + spriteInstancesCount, alignof(SpriteData));
	newSpriteInstanceData.material = (StringId64*)MemoryFn::alignTop(newSpriteInstanceData.sprite + spriteInstancesCount, alignof(StringId64));
	newSpriteInstanceData.frame = (uint32_t*)MemoryFn::alignTop(newSpriteInstanceData.material + spriteInstancesCount, alignof(uint32_t));
	newSpriteInstanceData.world = (Matrix4x4*)MemoryFn::alignTop(newSpriteInstanceData.frame + spriteInstancesCount, alignof(Matrix4x4));
	newSpriteInstanceData.aabb = (Aabb*)MemoryFn::alignTop(newSpriteInstanceData.world + spriteInstancesCount, alignof(Aabb));
	newSpriteInstanceData.nextInstance = (SpriteInstance*)MemoryFn::alignTop(newSpriteInstanceData.aabb + spriteInstancesCount, alignof(SpriteInstance));

	memcpy(newSpriteInstanceData.unit, this->data.unit, this->data.size * sizeof(UnitId));
	memcpy(newSpriteInstanceData.resource, this->data.resource, this->data.size * sizeof(SpriteResource**));
	memcpy(newSpriteInstanceData.sprite, this->data.sprite, this->data.size * sizeof(SpriteData));
	memcpy(newSpriteInstanceData.material, this->data.material, this->data.size * sizeof(StringId64));
	memcpy(newSpriteInstanceData.frame, this->data.frame, this->data.size * sizeof(uint32_t));
	memcpy(newSpriteInstanceData.world, this->data.world, this->data.size * sizeof(Matrix4x4));
	memcpy(newSpriteInstanceData.aabb, this->data.aabb, this->data.size * sizeof(Aabb));
	memcpy(newSpriteInstanceData.nextInstance, this->data.nextInstance, this->data.size * sizeof(SpriteInstance));

	allocator->deallocate(this->data.buffer);
	this->data = newSpriteInstanceData;
}

void RenderWorld::SpriteManager::grow()
{
	allocate(this->data.capacity * 2 + 1);
}

SpriteInstance RenderWorld::SpriteManager::create(UnitId id, const SpriteResource* spriteResource, StringId64 material, const Matrix4x4& transform)
{
	if (this->data.size == this->data.capacity)
	{
		grow();
	}

	const uint32_t last = this->data.size;

	this->data.unit[last] = id;
	this->data.resource[last] = spriteResource;
	this->data.sprite[last].vertexBufferHandle = spriteResource->vertexBufferHandle;
	this->data.sprite[last].indexBufferHandle = spriteResource->indexBufferHandle;
	this->data.material[last] = material;
	this->data.frame[last] = 0;
	this->data.world[last] = transform;
	this->data.aabb[last] = Aabb();
	this->data.nextInstance[last] = makeInstance(UINT32_MAX);

	++this->data.size;
	++this->data.firstHidden;

	HashMapFn::set(unitIdToSpriteMap, id, last);

	return makeInstance(last);
}

void RenderWorld::SpriteManager::destroy(SpriteInstance i)
{
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	const uint32_t last = this->data.size - 1;
	const UnitId unitId = this->data.unit[i.i];
	const UnitId lastUnitId = this->data.unit[last];

	this->data.unit[i.i] = this->data.unit[last];
	this->data.resource[i.i] = this->data.resource[last];
	this->data.sprite[i.i].vertexBufferHandle = this->data.sprite[last].vertexBufferHandle;
	this->data.sprite[i.i].indexBufferHandle = this->data.sprite[last].indexBufferHandle;
	this->data.material[i.i] = this->data.material[last];
	this->data.frame[i.i] = this->data.frame[last];
	this->data.world[i.i] = this->data.world[last];
	this->data.aabb[i.i] = this->data.aabb[last];
	this->data.nextInstance[i.i] = this->data.nextInstance[last];

	--this->data.size;
	--this->data.firstHidden;

	HashMapFn::set(unitIdToSpriteMap, lastUnitId, i.i);
	HashMapFn::remove(unitIdToSpriteMap, unitId);
}

bool RenderWorld::SpriteManager::has(UnitId id)
{
	return getIsValid(getSprite(id));
}

SpriteInstance RenderWorld::SpriteManager::getSprite(UnitId id)
{
	return makeInstance(HashMapFn::get(unitIdToSpriteMap, id, UINT32_MAX));
}

void RenderWorld::SpriteManager::destroy()
{
	allocator->deallocate(this->data.buffer);
}

void RenderWorld::LightManager::allocate(uint32_t lightInstancesCount)
{
	RIO_ENSURE(lightInstancesCount > this->data.size);

	const uint32_t bytes = 0
		+ lightInstancesCount * sizeof(UnitId) + alignof(UnitId)
		+ lightInstancesCount * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ lightInstancesCount * sizeof(float) + alignof(float)
		+ lightInstancesCount * sizeof(float) + alignof(float)
		+ lightInstancesCount * sizeof(float) + alignof(float)
		+ lightInstancesCount * sizeof(Color4) + alignof(Color4)
		+ lightInstancesCount * sizeof(uint32_t) + alignof(uint32_t)
		;

	LightInstanceData newLightInstanceData;
	newLightInstanceData.size = this->data.size;
	newLightInstanceData.capacity = lightInstancesCount;
	newLightInstanceData.buffer = allocator->allocate(bytes);

	newLightInstanceData.unit = (UnitId*)(newLightInstanceData.buffer);
	newLightInstanceData.world = (Matrix4x4*)MemoryFn::alignTop(newLightInstanceData.unit + lightInstancesCount, alignof(Matrix4x4));
	newLightInstanceData.range = (float*)MemoryFn::alignTop(newLightInstanceData.world + lightInstancesCount, alignof(float));
	newLightInstanceData.intensity = (float*)MemoryFn::alignTop(newLightInstanceData.range + lightInstancesCount, alignof(float));
	newLightInstanceData.spotAngle = (float*)MemoryFn::alignTop(newLightInstanceData.intensity + lightInstancesCount, alignof(float));
	newLightInstanceData.color = (Color4*)MemoryFn::alignTop(newLightInstanceData.spotAngle + lightInstancesCount, alignof(Color4));
	newLightInstanceData.type = (uint32_t*)MemoryFn::alignTop(newLightInstanceData.color + lightInstancesCount, alignof(uint32_t));

	memcpy(newLightInstanceData.unit, this->data.unit, this->data.size * sizeof(UnitId));
	memcpy(newLightInstanceData.world, this->data.world, this->data.size * sizeof(Matrix4x4));
	memcpy(newLightInstanceData.range, this->data.range, this->data.size * sizeof(float));
	memcpy(newLightInstanceData.intensity, this->data.intensity, this->data.size * sizeof(float));
	memcpy(newLightInstanceData.spotAngle, this->data.spotAngle, this->data.size * sizeof(float));
	memcpy(newLightInstanceData.color, this->data.color, this->data.size * sizeof(Color4));
	memcpy(newLightInstanceData.type, this->data.type, this->data.size * sizeof(uint32_t));

	allocator->deallocate(this->data.buffer);
	this->data = newLightInstanceData;
}

void RenderWorld::LightManager::grow()
{
	allocate(this->data.capacity * 2 + 1);
}

LightInstance RenderWorld::LightManager::create(UnitId id, const LightDesc& lightDesc, const Matrix4x4& transform)
{
	RIO_ASSERT(!HashMapFn::has(unitIdToLightMap, id), "Unit already has light");

	if (this->data.size == this->data.capacity)
	{
		grow();
	}

	const uint32_t last = this->data.size;

	this->data.unit[last] = id;
	this->data.world[last] = transform;
	this->data.range[last] = lightDesc.range;
	this->data.intensity[last] = lightDesc.intensity;
	this->data.spotAngle[last] = lightDesc.spotAngle;
	this->data.color[last] = createVector4(lightDesc.color.x, lightDesc.color.y, lightDesc.color.z, 1.0f);
	this->data.type[last] = lightDesc.type;

	++this->data.size;

	HashMapFn::set(unitIdToLightMap, id, last);
	return makeInstance(last);
}

void RenderWorld::LightManager::destroy(LightInstance i)
{
	RIO_ASSERT(i.i < this->data.size, "Index out of bounds");

	const uint32_t last = this->data.size - 1;
	const UnitId unitId = this->data.unit[i.i];
	const UnitId lastUnitId = this->data.unit[last];

	this->data.unit[i.i] = this->data.unit[last];
	this->data.world[i.i] = this->data.world[last];
	this->data.range[i.i] = this->data.range[last];
	this->data.intensity[i.i] = this->data.intensity[last];
	this->data.spotAngle[i.i] = this->data.spotAngle[last];
	this->data.color[i.i] = this->data.color[last];
	this->data.type[i.i] = this->data.type[last];

	--this->data.size;

	HashMapFn::set(unitIdToLightMap, lastUnitId, i.i);
	HashMapFn::remove(unitIdToLightMap, unitId);
}

bool RenderWorld::LightManager::has(UnitId id)
{
	return getIsValid(getLight(id));
}

LightInstance RenderWorld::LightManager::getLight(UnitId id)
{
	return makeInstance(HashMapFn::get(unitIdToLightMap, id, UINT32_MAX));
}

void RenderWorld::LightManager::destroy()
{
	allocator->deallocate(this->data.buffer);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka