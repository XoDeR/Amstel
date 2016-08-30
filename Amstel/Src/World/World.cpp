// Copyright (c) 2016 Volodymyr Syvochka
#include "World/World.h"

#include "Core/Error/Error.h"
#include "Core/Containers/HashMap.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"

#include "Resource/ResourceManager.h"
#include "Resource/UnitResource.h"

#include "World/DebugGui.h"
#include "World/DebugLine.h"
#include "World/Level.h"
#include "World/PhysicsWorld.h"
#include "World/RenderWorld.h"
#include "World/SceneGraph.h"
#include "World/SoundWorld.h"
#include "World/UnitManager.h"

#include "Script/ScriptEnvironment.h"

namespace Rio
{

World::World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager, ScriptEnvironment& ScriptEnvironment)
	: marker(WORLD_MARKER)
	, allocator(&a)
	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, scriptEnvironment(&ScriptEnvironment)
	, unitManager(&unitManager)
	, unitIdList(a)
	, levelList(a)
	, cameraList(a)
	, cameraMap(a)
	, eventStream(a)
{
	debugLine = createDebugLine(true);
	sceneGraph = RIO_NEW(*allocator, SceneGraph)(*allocator);
	renderWorld = RIO_NEW(*allocator, RenderWorld)(*allocator, resourceManager, shaderManager, materialManager, unitManager);
	physicsWorld = PhysicsWorldFn::create(*allocator, resourceManager, unitManager, *debugLine);
	soundWorld = SoundWorldFn::create(*allocator);
}

World::~World()
{
	// destroy members in reverse order
	destroyDebugLine(*debugLine);
	SoundWorldFn::destroy(*allocator, soundWorld);
	PhysicsWorldFn::destroy(*allocator, physicsWorld);
	RIO_DELETE(*allocator, renderWorld);
	RIO_DELETE(*allocator, sceneGraph);

	for (uint32_t i = 0; i < ArrayFn::getCount(levelList); ++i)
	{
		RIO_DELETE(*allocator, levelList[i]);
	}

	marker = 0;
}

UnitId World::spawnUnit(StringId64 name, const Vector3& position, const Quaternion& rotation)
{
	const UnitResource* unitResource = (const UnitResource*)resourceManager->get(RESOURCE_TYPE_UNIT, name);
	UnitId id = unitManager->create();
	spawnUnits(*this, *unitResource, position, rotation, &id);
	ArrayFn::pushBack(unitIdList, id);
	postUnitSpawnedEvent(id);
	return id;
}

UnitId World::spawnEmptyUnit()
{
	UnitId id = unitManager->create();
	ArrayFn::pushBack(unitIdList, id);
	postUnitSpawnedEvent(id);
	return id;
}

void World::destroyUnit(UnitId id)
{
	unitManager->destroy(id);
	postUnitDestroyedEvent(id);
}

uint32_t World::getUnitListCount() const
{
	return ArrayFn::getCount(unitIdList);
}

void World::getAllUnits(Array<UnitId>& unitIdList) const
{
	ArrayFn::reserve(unitIdList, ArrayFn::getCount(this->unitIdList));
	ArrayFn::push(unitIdList, ArrayFn::begin(this->unitIdList), ArrayFn::getCount(this->unitIdList));
}

void World::updateAnimations(float /*dt*/)
{
}

void World::updateScene(float dt)
{
	TempAllocator4096 ta;
	Array<UnitId> changedUnitList(ta);
	Array<Matrix4x4> changedWorldTransformList(ta);

	sceneGraph->getChanged(changedUnitList, changedWorldTransformList);

	physicsWorld->updateActorWorldPoses(ArrayFn::begin(changedUnitList)
		, ArrayFn::end(changedUnitList)
		, ArrayFn::begin(changedWorldTransformList)
		);

	physicsWorld->update(dt);

	// Process physics events
	EventStream& physicsEventStream = physicsWorld->getEventStream();

	const uint32_t size = ArrayFn::getCount(physicsEventStream);
	uint32_t read = 0;
	while (read < size)
	{
		const EventHeader* eventHeader = (EventHeader*)&physicsEventStream[read];
		const char* data = (char*)&eventHeader[1];

		read += sizeof(eventHeader) + eventHeader->size;

		switch (eventHeader->type)
		{
		case EventType::PHYSICS_TRANSFORM:
		{
			const PhysicsTransformEvent& physicsTransformEvent = *(PhysicsTransformEvent*)data;
			const TransformInstance transformInstance = sceneGraph->get(physicsTransformEvent.unitId);
			const Matrix4x4 pose = createMatrix4x4(physicsTransformEvent.rotation, physicsTransformEvent.position);
			sceneGraph->setWorldPose(transformInstance, pose);
		}
		break;
		case EventType::PHYSICS_COLLISION:
			break;
		case EventType::PHYSICS_TRIGGER:
			break;
		default:
			RIO_FATAL("Unknown event type");
			break;
		}
	}
	ArrayFn::clear(physicsEventStream);

	ArrayFn::clear(changedUnitList);
	ArrayFn::clear(changedWorldTransformList);

	sceneGraph->getChanged(changedUnitList, changedWorldTransformList);
	sceneGraph->clearChanged();

	renderWorld->updateTransforms(ArrayFn::begin(changedUnitList)
		, ArrayFn::end(changedUnitList)
		, ArrayFn::begin(changedWorldTransformList)
		);

	soundWorld->update();

	ArrayFn::clear(eventStream);
}

void World::update(float dt)
{
	updateAnimations(dt);
	updateScene(dt);
}

void World::render(const Matrix4x4& view, const Matrix4x4& projection)
{
	renderWorld->render(view, projection);

	physicsWorld->debugDraw();
	renderWorld->debugDraw(*debugLine);

	debugLine->submit();
	debugLine->reset();
}

CameraInstance World::cameraCreate(UnitId id, const CameraDesc& cameraDesc, const Matrix4x4& /*transformMatrix4x4*/)
{
	Camera camera;
	camera.unit = id;
	camera.projectionType = (ProjectionType::Enum)cameraDesc.type;
	camera.fov = cameraDesc.fov;
	camera.near = cameraDesc.nearRange;
	camera.far = cameraDesc.farRange;

	const uint32_t last = ArrayFn::getCount(cameraList);
	ArrayFn::pushBack(cameraList, camera);

	HashMapFn::set(cameraMap, id, last);
	return makeCameraInstance(last);
}

void World::cameraDestroy(CameraInstance i)
{
	const uint32_t last = ArrayFn::getCount(cameraList) - 1;
	const UnitId unitId = cameraList[i.i].unit;
	const UnitId lastUnitId = cameraList[last].unit;

	cameraList[i.i] = cameraList[last];

	HashMapFn::set(cameraMap, lastUnitId, i.i);
	HashMapFn::remove(cameraMap, unitId);
}

CameraInstance World::cameraGet(UnitId id)
{
	return makeCameraInstance(HashMapFn::get(cameraMap, id, UINT32_MAX));
}

void World::cameraSetProjectionType(CameraInstance i, ProjectionType::Enum type)
{
	cameraList[i.i].projectionType = type;
	cameraList[i.i].updateProjectionMatrix();
}

ProjectionType::Enum World::cameraGetProjectionType(CameraInstance i) const
{
	return cameraList[i.i].projectionType;
}

const Matrix4x4& World::cameraGetProjectionMatrix(CameraInstance i) const
{
	return cameraList[i.i].projection;
}

Matrix4x4 World::cameraGetViewMatrix(CameraInstance i) const
{
	const TransformInstance transformInstance = sceneGraph->get(cameraList[i.i].unit);
	Matrix4x4 view = sceneGraph->getWorldPose(transformInstance);
	invert(view);
	return view;
}

float World::cameraGetFov(CameraInstance i) const
{
	return cameraList[i.i].fov;
}

void World::cameraSetFov(CameraInstance i, float fov)
{
	cameraList[i.i].fov = fov;
	cameraList[i.i].updateProjectionMatrix();
}

void World::cameraSetAspect(CameraInstance i, float aspect)
{
	cameraList[i.i].aspect = aspect;
	cameraList[i.i].updateProjectionMatrix();
}

float World::cameraGetNearClipDistance(CameraInstance i) const
{
	return cameraList[i.i].near;
}

void World::cameraSetNearClipDistance(CameraInstance i, float near)
{
	cameraList[i.i].near = near;
	cameraList[i.i].updateProjectionMatrix();
}

float World::cameraGetFarClipDistance(CameraInstance i) const
{
	return cameraList[i.i].far;
}

void World::cameraSetFarClipDistance(CameraInstance i, float far)
{
	cameraList[i.i].far = far;
	cameraList[i.i].updateProjectionMatrix();
}

void World::cameraSetOrthographicMetrics(CameraInstance i, float left, float right, float bottom, float top)
{
	cameraList[i.i].left = left;
	cameraList[i.i].right = right;
	cameraList[i.i].bottom = bottom;
	cameraList[i.i].top = top;

	cameraList[i.i].updateProjectionMatrix();
}

void World::cameraSetViewportMetrics(CameraInstance i, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	cameraList[i.i].viewX = x;
	cameraList[i.i].viewY = y;
	cameraList[i.i].viewWidth = width;
	cameraList[i.i].viewHeight = height;
}

Vector3 World::cameraGetWorldFromScreen(CameraInstance i, const Vector3& position)
{
	const Camera& camera = cameraList[i.i];

	const TransformInstance transformInstance = sceneGraph->get(cameraList[i.i].unit);
	Matrix4x4 worldInverse = sceneGraph->getWorldPose(transformInstance);
	invert(worldInverse);
	// model view projection matrix
	Matrix4x4 mvp = worldInverse * camera.projection;
	invert(mvp);

	// normalized device coordinates
	Vector4 ndc;
	ndc.x = (2.0f * (position.x - 0.0f)) / camera.viewWidth - 1.0f;
	ndc.y = (2.0f * (camera.viewHeight - position.y)) / camera.viewHeight - 1.0f;
	ndc.z = (2.0f * position.z) - 1.0f;
	ndc.w = 1.0f;

	Vector4 tmp = ndc * mvp;
	tmp *= 1.0f / tmp.w;

	return createVector3(tmp.x, tmp.y, tmp.z);
}

Vector3 World::cameraGetScreenFromWorld(CameraInstance i, const Vector3& position)
{
	const Camera& camera = cameraList[i.i];

	const TransformInstance transformInstance = sceneGraph->get(cameraList[i.i].unit);
	Matrix4x4 worldInverse = sceneGraph->getWorldPose(transformInstance);
	invert(worldInverse);

	Vector4 xyzw;
	xyzw.x = position.x;
	xyzw.y = position.y;
	xyzw.z = position.z;
	xyzw.w = 1.0f;

	Vector4 clip = xyzw * (worldInverse * camera.projection);

	// normalized device coordinates
	Vector4 ndc;
	ndc.x = clip.x / clip.w;
	ndc.y = clip.y / clip.w;

	Vector3 screen;
	screen.x = (camera.viewX + camera.viewWidth  * (ndc.x + 1.0f)) / 2.0f;
	screen.y = (camera.viewY + camera.viewHeight * (1.0f - ndc.y)) / 2.0f;
	screen.z = 0.0f;

	return screen;
}

SoundInstanceId World::playSound(const SoundResource& soundResource, const bool loop, const float volume, const Vector3& position, const float range)
{
	return soundWorld->play(soundResource, loop, volume, range, position);
}

SoundInstanceId World::playSound(StringId64 name, const bool loop, const float volume, const Vector3& position, const float range)
{
	const SoundResource* soundResource = (const SoundResource*)resourceManager->get(RESOURCE_TYPE_SOUND, name);
	return playSound(*soundResource, loop, volume, position, range);
}

void World::stopSound(SoundInstanceId id)
{
	soundWorld->stop(id);
}

void World::linkSound(SoundInstanceId /*soundInstanceId*/, UnitId /*unitId*/, int32_t /*node*/)
{
	RIO_FATAL("Not implemented yet");
}

void World::setListenerPose(const Matrix4x4& pose)
{
	soundWorld->setListenerPose(pose);
}

void World::setSoundPosition(SoundInstanceId id, const Vector3& position)
{
	soundWorld->setSoundPositions(1, &id, &position);
}

void World::setSoundRange(SoundInstanceId id, float range)
{
	soundWorld->setSoundRanges(1, &id, &range);
}

void World::setSoundVolume(SoundInstanceId id, float vol)
{
	soundWorld->setSoundVolumes(1, &id, &vol);
}

DebugLine* World::createDebugLine(bool depthTest)
{
	return RIO_NEW(*allocator, DebugLine)(depthTest);
}

void World::destroyDebugLine(DebugLine& line)
{
	RIO_DELETE(*allocator, &line);
}

DebugGui* World::createScreenDebugGui(float scaleWidth, float scaleHeight)
{
	// TODO use scaleWidth and scaleHeight
	scaleWidth = 1280.0f;
	scaleHeight = 720.0f;

	return RIO_NEW(*allocator, DebugGui)(*resourceManager
		, *shaderManager
		, *materialManager
		, scaleWidth
		, scaleHeight
		);
}

void World::destroyGui(DebugGui& debugGui)
{
	RIO_DELETE(*allocator, &debugGui);
}

Level* World::loadLevel(StringId64 name, const Vector3& position, const Quaternion& rotation)
{
	const LevelResource* levelResource = (const LevelResource*)resourceManager->get(RESOURCE_TYPE_LEVEL, name);

	Level* level = RIO_NEW(*allocator, Level)(*allocator, *unitManager, *this, *levelResource);
	level->load(position, rotation);

	ArrayFn::pushBack(levelList, level);
	postLevelLoadedEvent();

	return level;
}

EventStream& World::getEventStream()
{
	return eventStream;
}

SceneGraph* World::getSceneGraph()
{
	return sceneGraph;
}

RenderWorld* World::getRenderWorld()
{
	return renderWorld;
}

PhysicsWorld* World::getPhysicsWorld()
{
	return physicsWorld;
}

SoundWorld* World::getSoundWorld()
{
	return soundWorld;
}

void World::postUnitSpawnedEvent(UnitId id)
{
	UnitSpawnedEvent ev;
	ev.unit = id;
	EventStreamFn::write(eventStream, EventType::UNIT_SPAWNED, ev);
}

void World::postUnitDestroyedEvent(UnitId id)
{
	UnitDestroyedEvent ev;
	ev.unit = id;
	EventStreamFn::write(eventStream, EventType::UNIT_DESTROYED, ev);
}

void World::postLevelLoadedEvent()
{
	LevelLoadedEvent ev;
	EventStreamFn::write(eventStream, EventType::LEVEL_LOADED, ev);
}

void World::Camera::updateProjectionMatrix()
{
	switch (projectionType)
	{
		case ProjectionType::ORTHOGRAPHIC:
		{
			setToOrthographic(projection
				, left
				, right
				, bottom
				, top
				, near
				, far
				);
		}
		break;
		case ProjectionType::PERSPECTIVE:
		{
			setToPerspective(projection
				, fov
				, aspect
				, near
				, far
				);
		}
		break;
		default:
		{
			RIO_FATAL("Error: unknown projection type");
		}
		break;
	}
}

void spawnUnits(World& world, const UnitResource& unitResource, const Vector3& position, const Quaternion& rotation, const UnitId* unitLookupList)
{
	SceneGraph* sceneGraph = world.getSceneGraph();
	RenderWorld* renderWorld = world.getRenderWorld();
	PhysicsWorld* physicsWorld = world.getPhysicsWorld();

	// Start of components data
	const char* componentListBegin = (const char*)(&unitResource + 1);
	const ComponentData* component = nullptr;

	for (uint32_t componentTypeListIndex = 0; componentTypeListIndex < unitResource.componentTypesCount; ++componentTypeListIndex, componentListBegin += component->size + sizeof(ComponentData))
	{
		component = (const ComponentData*)componentListBegin;
		const uint32_t* unitIndexList = (const uint32_t*)(component + 1);
		const char* data = (const char*)(unitIndexList + component->instancesCount);

		if (component->type == COMPONENT_TYPE_TRANSFORM)
		{
			const TransformDesc* transformDesc = (const TransformDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++transformDesc)
			{
				Matrix4x4 matrix = createMatrix4x4(rotation, position);
				Matrix4x4 matrix_res = createMatrix4x4(transformDesc->rotation, transformDesc->position);
				sceneGraph->create(unitLookupList[unitIndexList[i]], matrix_res*matrix);
			}
		}
		else if (component->type == COMPONENT_TYPE_CAMERA)
		{
			const CameraDesc* cameraDesc = (const CameraDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++cameraDesc)
			{
				world.cameraCreate(unitLookupList[unitIndexList[i]], *cameraDesc, MATRIX4X4_IDENTITY);
			}
		}
		else if (component->type == COMPONENT_TYPE_COLLIDER)
		{
			const ColliderDesc* colliderDesc = (const ColliderDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i)
			{
				physicsWorld->colliderCreate(unitLookupList[unitIndexList[i]], colliderDesc);
				colliderDesc = (ColliderDesc*)((char*)(colliderDesc + 1) + colliderDesc->size);
			}
		}
		else if (component->type == COMPONENT_TYPE_ACTOR)
		{
			const ActorResource* actorResource = (const ActorResource*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++actorResource)
			{
				Matrix4x4 transformMatrix = sceneGraph->getWorldPose(sceneGraph->get(unitLookupList[unitIndexList[i]]));
				physicsWorld->actorCreate(unitLookupList[unitIndexList[i]], actorResource, transformMatrix);
			}
		}
		else if (component->type == COMPONENT_TYPE_CONTROLLER)
		{
			const ControllerDesc* controllerDesc = (const ControllerDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++controllerDesc)
			{
				Matrix4x4 transformMatrix = sceneGraph->getWorldPose(sceneGraph->get(unitLookupList[unitIndexList[i]]));
				physicsWorld->controllerCreate(unitLookupList[unitIndexList[i]], *controllerDesc, transformMatrix);
			}
		}
		else if (component->type == COMPONENT_TYPE_MESH_RENDERER)
		{
			const MeshRendererDesc* meshRendererDesc = (const MeshRendererDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++meshRendererDesc)
			{
				Matrix4x4 transformMatrix = sceneGraph->getWorldPose(sceneGraph->get(unitLookupList[unitIndexList[i]]));
				renderWorld->meshCreate(unitLookupList[unitIndexList[i]], *meshRendererDesc, transformMatrix);
			}
		}
		else if (component->type == COMPONENT_TYPE_SPRITE_RENDERER)
		{
			const SpriteRendererDesc* spriteRendererDesc = (const SpriteRendererDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++spriteRendererDesc)
			{
				Matrix4x4 transformMatrix = sceneGraph->getWorldPose(sceneGraph->get(unitLookupList[unitIndexList[i]]));
				renderWorld->spriteCreate(unitLookupList[unitIndexList[i]], *spriteRendererDesc, transformMatrix);
			}
		}
		else if (component->type == COMPONENT_TYPE_LIGHT)
		{
			const LightDesc* lightDesc = (const LightDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++lightDesc)
			{
				Matrix4x4 transformMatrix = sceneGraph->getWorldPose(sceneGraph->get(unitLookupList[unitIndexList[i]]));
				renderWorld->lightCreate(unitLookupList[unitIndexList[i]], *lightDesc, transformMatrix);
			}
		}
		else
		{
			RIO_FATAL("Unknown component type");
		}
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka