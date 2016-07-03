// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/EventStream.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"
#include "Script/ScriptTypes.h"

namespace Rio
{

class World
{
private:
	struct Camera
	{
		UnitId unit;

		ProjectionType::Enum projectionType;
		Matrix4x4 projection;

		Frustum frustum;
		float fov;
		float aspect;
		float near;
		float far;

		// Orthographic projection only
		float left;
		float right;
		float bottom;
		float top;

		uint16_t viewX;
		uint16_t viewY;
		uint16_t viewWidth;
		uint16_t viewHeight;

		void updateProjectionMatrix();
	};
public:
	World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager, ScriptEnvironment& ScriptEnvironment);
	~World();

	UnitId spawnUnit(StringId64 name, const Vector3& position = VECTOR3_ZERO, const Quaternion& rotation = QUATERNION_IDENTITY);
	UnitId spawnEmptyUnit();
	void destroyUnit(UnitId id);

	// Returns the number of units in the world
	uint32_t getUnitListCount() const;

	// Returns all the the units in the world
	void getAllUnits(Array<UnitId>& units) const;

	// Creates a new camera.
	CameraInstance createCamera(UnitId id, const CameraDesc& cameraDesc);
	// Destroys the camera <cameraInstance>
	void destroyCamera(CameraInstance cameraInstance);
	// Returns the camera owned by unit <id>
	CameraInstance getCamera(UnitId id);
	// Sets the projection type of the camera
	void setCameraProjectionType(CameraInstance i, ProjectionType::Enum type);
	ProjectionType::Enum getCameraProjectionType(CameraInstance i) const;
	// Returns the projection matrix of the camera
	const Matrix4x4& getCameraProjectionMatrix(CameraInstance i) const;
	// Returns the view matrix of the camera
	Matrix4x4 getCameraViewMatrix(CameraInstance i) const;
	// Returns the field-of-view of the camera in degrees
	float getCameraFov(CameraInstance i) const;
	// Sets the field-of-view of the camera in degrees
	void setCameraFov(CameraInstance i, float fov);
	// Returns the aspect ratio of the camera (Perspective projection only)
	float getCameraAspect(CameraInstance i) const;
	// Sets the aspect ratio of the camera (Perspective projection only)
	void setCameraAspect(CameraInstance i, float aspect);
	// Returns the near clip distance of the camera
	float getCameraNearClipDistance(CameraInstance i) const;
	// Sets the near clip distance of the camera
	void setCameraNearClipDistance(CameraInstance i, float near);
	// Returns the far clip distance of the camera
	float getCameraFarClipDistance(CameraInstance i) const;
	// Sets the far clip distance of the camera
	void setCameraFarClipDistance(CameraInstance i, float far);
	// Sets the coordinates for orthographic clipping planes
	// (Orthographic projection only)
	void setCameraOrthographicMetrics(CameraInstance i, float left, float right, float bottom, float top);
	// Sets the coordinates for the camera viewport in pixels
	void setCameraViewportMetrics(CameraInstance i, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
	// Returns <position> from screen-space to world-space coordinates
	Vector3 getCameraWorldFromScreen(CameraInstance i, const Vector3& position);
	// Returns <position> from world-space to screen-space coordinates
	Vector3 getCameraScreenFromWorld(CameraInstance i, const Vector3& position);
	
	void updateAnimations(float dt);
	void updateScene(float dt);
	void update(float dt);
	void render(CameraInstance i);

	SoundInstanceId playSound(const SoundResource& soundResource, bool loop = false, float volume = 1.0f, const Vector3& position = VECTOR3_ZERO, float range = 50.0f);

	// Plays the sound with the given <name> at the given <position>, with the given <volume> and <range>
	// <loop> controls whether the sound must loop or not
	SoundInstanceId playSound(StringId64 name, const bool loop, const float volume, const Vector3& position, const float range);

	// Stops the sound with the given <id>
	void stopSound(SoundInstanceId id);

	// Links the sound <id> to the <node> of the given <unit>
	// After this call, the sound <id> will follow the unit <unit>
	void linkSound(SoundInstanceId id, UnitId unit, int32_t node);

	// Sets the <pose> of the listener
	void setListenerPose(const Matrix4x4& pose);

	// Sets the <position> of the sound <id>
	void setSoundPosition(SoundInstanceId id, const Vector3& position);

	// Sets the <range> of the sound <id>
	void setSoundRange(SoundInstanceId id, float range);

	// Sets the <volume> of the sound <id>
	void setSoundVolume(SoundInstanceId id, float volume);

	// Creates a new DebugLine
	// <depthTest> controls whether to
	// enable depth test when rendering the lines
	DebugLine* createDebugLine(bool depthTest);
	void destroyDebugLine(DebugLine& line);

	// Creates a new screen-space Gui
	Gui* createScreenGui(float scaleWidth, float scaleHeight);
	void destroyGui(Gui& gui);

	// Loads the level <name> into the world
	Level* loadLevel(StringId64 name, const Vector3& position, const Quaternion& rotation);

	// Returns the events.
	EventStream& getEventStream();
	
	SceneGraph* getSceneGraph();
	RenderWorld* getRenderWorld();
	PhysicsWorld* getPhysicsWorld();
	SoundWorld* getSoundWorld();

	void postUnitSpawnedEvent(UnitId id);
	void postUnitDestroyedEvent(UnitId id);
	void postLevelLoadedEvent();

	static const uint32_t MARKER = 0xfb6ce2d3;
private:
	uint32_t marker;

	Allocator* allocator;
	ResourceManager* resourceManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	ScriptEnvironment* scriptEnvironment;
	UnitManager* unitManager;

	DebugLine* debugLine = nullptr;
	SceneGraph* sceneGraph = nullptr;
	RenderWorld* renderWorld = nullptr;
	PhysicsWorld* physicsWorld = nullptr;
	SoundWorld* soundWorld = nullptr;

	Array<UnitId> unitIdList;
	Array<Level*> levelList;
	Array<Camera> cameraList;
	HashMap<UnitId, uint32_t> cameraMap;

	EventStream eventStream;

	CameraInstance makeCameraInstance(uint32_t i) 
	{ 
		CameraInstance cameraInstance = { i };
		return cameraInstance;
	}
};

void spawnUnits(World& world, const UnitResource& unitResource, const Vector3& position, const Quaternion& rotation, const UnitId* unitLookupList);

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka