// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Math/MathTypes.h"
#include "Core/Base/Functional.h"
#include "Core/Strings/StringId.h"

namespace Rio
{

class Level;
class MaterialManager;
class PhysicsWorld;
class RenderWorld;
class ShaderManager;
class SoundWorld;
class UnitManager;
class World;
struct DebugLine;
struct Gui;
struct Material;
struct SceneGraph;

using SoundInstanceId = uint32_t;

const StringId32 COMPONENT_TYPE_TRANSFORM = StringId32("transform");
const StringId32 COMPONENT_TYPE_CAMERA = StringId32("camera");
const StringId32 COMPONENT_TYPE_COLLIDER = StringId32("collider");
const StringId32 COMPONENT_TYPE_ACTOR = StringId32("actor");
const StringId32 COMPONENT_TYPE_CONTROLLER = StringId32("controller");
const StringId32 COMPONENT_TYPE_MESH_RENDERER = StringId32("meshRenderer");
const StringId32 COMPONENT_TYPE_SPRITE_RENDERER = StringId32("spriteRenderer");
const StringId32 COMPONENT_TYPE_LIGHT = StringId32("light");

// Camera projection types.
struct ProjectionType
{
	enum Enum
	{
		ORTHOGRAPHIC,
		PERSPECTIVE,

		COUNT
	};
};

struct LightType
{
	enum Enum
	{
		DIRECTIONAL,
		OMNI,
		SPOT,

		COUNT
	};
};

struct ActorType
{
	enum Enum
	{
		STATIC,
		DYNAMIC_PHYSICAL,
		DYNAMIC_KINEMATIC,

		COUNT
	};
};

struct ActorFlags
{
	enum Enum
	{
		LOCK_TRANSLATION_X = 1 << 0,
		LOCK_TRANSLATION_Y = 1 << 1,
		LOCK_TRANSLATION_Z = 1 << 2,
		LOCK_ROTATION_X    = 1 << 3,
		LOCK_ROTATION_Y    = 1 << 4,
		LOCK_ROTATION_Z    = 1 << 5
	};
};

struct ColliderType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		CONVEX_HULL,
		MESH,
		HEIGHTFIELD,

		COUNT
	};
};

struct JointType
{
	enum Enum
	{
		FIXED,
		HINGE,
		SPRING,

		COUNT
	};
};

struct CollisionGroup
{
	enum Enum
	{
		GROUP_0  = 1 <<  0, // Reserved
		GROUP_1  = 1 <<  1,
		GROUP_2  = 1 <<  2,
		GROUP_3  = 1 <<  3,
		GROUP_4  = 1 <<  4,
		GROUP_5  = 1 <<  5,
		GROUP_6  = 1 <<  6,
		GROUP_7  = 1 <<  7,
		GROUP_8  = 1 <<  8,
		GROUP_9  = 1 <<  9,
		GROUP_10 = 1 << 10,
		GROUP_11 = 1 << 11,
		GROUP_12 = 1 << 12,
		GROUP_13 = 1 << 13,
		GROUP_14 = 1 << 14,
		GROUP_15 = 1 << 15,
		GROUP_16 = 1 << 16,
		GROUP_17 = 1 << 17,
		GROUP_18 = 1 << 18,
		GROUP_19 = 1 << 19,
		GROUP_20 = 1 << 20,
		GROUP_21 = 1 << 21,
		GROUP_22 = 1 << 22,
		GROUP_23 = 1 << 23,
		GROUP_24 = 1 << 24,
		GROUP_25 = 1 << 25,
		GROUP_26 = 1 << 26,
		GROUP_27 = 1 << 27,
		GROUP_28 = 1 << 28,
		GROUP_29 = 1 << 29,
		GROUP_30 = 1 << 30,
		GROUP_31 = 1 << 31
	};
};

struct RaycastMode
{
	enum Enum
	{
		CLOSEST,
		ALL,

		COUNT
	};
};

struct EventType
{
	enum Enum
	{
		UNIT_SPAWNED,
		UNIT_DESTROYED,

		LEVEL_LOADED,

		PHYSICS_COLLISION,
		PHYSICS_TRIGGER,
		PHYSICS_TRANSFORM,

		COUNT
	};
};

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS 8
#define UNIT_ID_MASK 0x3fc00000

struct UnitId
{
	uint32_t getIndex() const
	{
		return index & UNIT_INDEX_MASK;
	}

	uint32_t getId() const
	{
		return (index >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
	}

	bool getIsValid()
	{
		return index != UINT32_MAX;
	}

	uint32_t index;
};

inline bool operator==(const UnitId& a, const UnitId& b)
{
	return a.index == b.index;
}

const UnitId UNIT_INVALID = { UINT32_MAX };

template <>
struct THash<UnitId>
{
	uint32_t operator()(const UnitId& id) const
	{
		return id.index;
	}
};

struct TransformInstance
{
	uint32_t i;
};

struct CameraInstance
{
	uint32_t i;
};

struct MeshInstance
{
	uint32_t i;
};

struct SpriteInstance
{
	uint32_t i;
};

struct LightInstance
{
	uint32_t i;
};

struct ColliderInstance
{
	uint32_t i;
};

struct ActorInstance
{
	uint32_t i;
};

struct ControllerInstance
{
	uint32_t i;
};

struct JointInstance
{
	uint32_t i;
};

// Mesh renderer description.
struct MeshRendererDesc
{
	StringId64 meshResource; // Name of .mesh resource
	StringId32 geometryName; // Name of geometry inside .mesh resource
	StringId64 materialResource; // Name of .material resource
	bool visible; // Whether mesh is visible
	char _pad[3];
};

// Sprite renderer description.
struct SpriteRendererDesc
{
	StringId64 spriteResourceName;   // Name of .sprite resource
	StringId64 materialResource; // Name of .material resource
	bool visible; // Whether sprite is visible
	char _pad[3];
	char _pad1[4];
};

// Light description.
struct LightDesc
{
	uint32_t type; // LightType::Enum
	float range; // In meters
	float intensity;
	float spotAngle; // In radians
	Vector3 color;  // Color of the light
};

// Transform description.
struct TransformDesc
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};

// Camera description
struct CameraDesc
{
	uint32_t type;  // ProjectionType::Enum
	float fov; // Vertical FOV
	float nearRange; // Near clipping plane distance
	float farRange; // Far clipping plane distance
};

// Controller description
struct ControllerDesc
{
	float height; // Height of the capsule
	float radius; // Radius of the capsule
	float slopeLimit; // The maximum slope which the character can walk up in radians
	float stepOffset; // Maximum height of an obstacle which the character can climb
	float contactOffset; // Skin around the object within which contacts will be generated. Use it to avoid numerical precision issues
	StringId32 collisionFilter; // Collision filter from global.physicsConfig
};

// Actor resource
struct ActorResource
{
	StringId32 actorClass; // Name of actor in global.physics resource
	float mass; // Mass of the actor
	uint32_t flags; // ActorFlags::Enum
	StringId32 collisionFilter; // Name of collision filter in global.physicsConfig resource
};

struct SphereShape
{
	float radius;
};

struct CapsuleShape
{
	float radius;
	float height;
};

struct BoxShape
{
	Vector3 halfSize;
};

struct HeightfieldShape
{
	uint32_t width;
	uint32_t length;
	float heightScale;
	float minHeight;
	float maxHeight;
};

struct ColliderDesc
{
	uint32_t type; // ShapeType::Enum
	StringId32 shapeClass; // Name of shape in global.physicsConfig resource
	StringId32 material; // Name of material in global.physicsConfig resource
	Matrix4x4 localTransformMatrix; // In actor-space
	SphereShape sphere;
	CapsuleShape capsule;
	BoxShape box;
	HeightfieldShape heightfield;
	uint32_t size; // Size of additional data
//	char data[size] // Convex Hull, Mesh, Heightfield data
};

struct HingeJoint
{
	Vector3 axis;

	bool useMotor;
	float targetVelocity;
	float maxMotorImpulse;

	bool useLimits;
	float lowerLimit;
	float upperLimit;
	float bounciness;
};

struct JointDesc
{
	uint32_t type; // JointType::Enum
	Vector3 anchor0;
	Vector3 anchor1;

	bool breakable;
	char _pad[3];
	float breakForce;

	HingeJoint hinge;
};

struct RaycastHit
{
	ActorInstance actor;
	Vector3 position; // In world-space
	Vector3 normal; // In world-space
};

struct UnitSpawnedEvent
{
	UnitId unit; // The unit spawned.
};

struct UnitDestroyedEvent
{
	UnitId unit; // The unit destroyed.
};

struct LevelLoadedEvent
{
};

struct PhysicsCollisionEvent
{
	enum Type { BEGIN_TOUCH, END_TOUCH } type;
	ActorInstance actors[2];
	Vector3 where; // In world-space
	Vector3 normal; // In world-space
};

struct PhysicsTriggerEvent
{
	enum Type 
	{ 
		BEGIN_TOUCH,
		END_TOUCH 
	} type;
	ActorInstance trigger;
	ActorInstance other;
};

struct PhysicsTransformEvent
{
	UnitId unitId;
	Vector3 position; // In world-space
	Quaternion rotation; // In world-space
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka