// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/EventStream.h"
#include "Core/Math/MathTypes.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

namespace Rio
{

class PhysicsWorld
{
public:
	PhysicsWorld() {}
	virtual ~PhysicsWorld() {}

	virtual ColliderInstance colliderCreate(UnitId id, const ColliderDesc* colliderDesc) = 0;
	virtual void colliderDestroy(ColliderInstance colliderInstance) = 0;
	virtual ColliderInstance colliderGetFirst(UnitId id) = 0;
	virtual ColliderInstance colliderGetNext(ColliderInstance colliderInstance) = 0;

	virtual ActorInstance actorCreate(UnitId id, const ActorResource* actorResource, const Matrix4x4& transformMatrix) = 0;
	virtual void actorDestroy(ActorInstance i) = 0;
	virtual ActorInstance actorGet(UnitId id) = 0;
	// Returns the world position of the actor
	virtual Vector3 actorGetWorldPosition(ActorInstance i) const = 0;
	// Returns the world rotation of the actor
	virtual Quaternion actorGetWorldRotation(ActorInstance i) const = 0;
	// Returns the world pose of the actor
	virtual Matrix4x4 actorGetWorldPose(ActorInstance i) const = 0;
	// Teleports the actor to the given world position
	virtual void actorTeleportWorldPosition(ActorInstance i, const Vector3& p) = 0;
	// Teleports the actor to the given world rotation
	virtual void actorTeleportWorldRotation(ActorInstance i, const Quaternion& r) = 0;
	// Teleports the actor to the given world pose
	virtual void actorTeleportWorldPose(ActorInstance i, const Matrix4x4& m) = 0;
	// Returns the center of mass of the actor
	virtual Vector3 actorGetCenterOfMass(ActorInstance i) const = 0;
	// Enables gravity for the actor
	virtual void actorEnableGravity(ActorInstance i) = 0;
	// Disables gravity for the actor
	virtual void actorDisableGravity(ActorInstance i) = 0;
	// Enables collision detection for the actor
	virtual void actorEnableCollision(ActorInstance i) = 0;
	// Disables collision detection for the actor
	virtual void actorDisableCollision(ActorInstance i) = 0;
	// Sets the collision filter of the actor
	virtual void actorSetCollisionFilter(ActorInstance i, StringId32 filter) = 0;
	// Sets whether the actor is kinematic or not
	// This call has no effect on static actors
	virtual void actorSetKinematic(ActorInstance i, bool kinematic) = 0;
	// Moves the actor to <position>
	// This call only affects nonkinematic actors
	virtual void actorMove(ActorInstance i, const Vector3& position) = 0;
	virtual bool actorGetIsStatic(ActorInstance i) const = 0;
	virtual bool actorGetIsDynamic(ActorInstance i) const = 0;
	// Returns whether the actor is kinematic (keyframed)
	virtual bool actorGetIsKinematic(ActorInstance i) const = 0;
	// Returns whether the actor is nonkinematic (dynamic and not kinematic)
	virtual bool actorGetIsNonKinematic(ActorInstance i) const = 0;
	// Returns the linear damping of the actor
	virtual float actorGetLinearDamping(ActorInstance i) const = 0;
	// Sets the linear damping of the actor
	virtual void actorSetLinearDamping(ActorInstance i, float rate) = 0;
	// Returns the angular damping of the actor
	virtual float actorGetAngularDamping(ActorInstance i) const = 0;
	// Sets the angular damping of the actor
	virtual void actorSetAngularDamping(ActorInstance i, float rate) = 0;
	// Returns the linear velocity of the actor
	virtual Vector3 actorGetLinearVelocity(ActorInstance i) const = 0;
	// Sets the linear velocity of the actor
	// This call only affects nonkinematic actors
	virtual void actorSetLinearVelocity(ActorInstance i, const Vector3& velocity) = 0;
	virtual Vector3 actorGetAngularVelocity(ActorInstance i) const = 0;
	// Sets the angular velocity of the actor
	// This call only affects nonkinematic actors
	virtual void actorSetAngularVelocity(ActorInstance i, const Vector3& velocity) = 0;
	// Adds a linear impulse (acting along the center of mass) to the actor
	// This call only affects nonkinematic actors
	virtual void actorAddImpulse(ActorInstance i, const Vector3& impulse) = 0;
	// Adds a linear impulse (acting along the world <position>) to the actor
	// This call only affects nonkinematic actors
	virtual void actorAddImpulseAt(ActorInstance i, const Vector3& impulse, const Vector3& position) = 0;
	// Adds a torque impulse to the actor
	virtual void actorAddTorqueImpulse(ActorInstance i, const Vector3& impulse) = 0;
	// Pushes the actor as if it was hit by a point object with the given <mass>
	// travelling at the given <velocity>
	// This call only affects nonkinematic actors
	virtual void actorPush(ActorInstance i, const Vector3& velocity, float mass) = 0;
	// Like push() but applies the force at the world position <position>
	// This call only affects nonkinematic actors
	virtual void actorPushAt(ActorInstance i, const Vector3& velocity, float mass, const Vector3& position) = 0;
	virtual bool actorGetIsSleeping(ActorInstance i) = 0;
	virtual void actorWakeUp(ActorInstance i) = 0;

	virtual ControllerInstance controllerCreate(UnitId id, const ControllerDesc& controllerDesc, const Matrix4x4& transformMatrix) = 0;
	virtual void controllerDestroy(ControllerInstance id) = 0;
	virtual ControllerInstance controllerGet(UnitId id) = 0;
	// Returns the position of the controller
	virtual Vector3 controllerGetPosition(ControllerInstance i) const = 0;
	// Moves the controller to <position>
	virtual void controllerMove(ControllerInstance i, const Vector3& position) = 0;
	// Sets the contoller height
	virtual void controllerSetHeight(ControllerInstance i, float height) = 0;
	// Returns whether the controller collides upwards
	virtual bool controllerDoesCollideUp(ControllerInstance i) const = 0;
	// Returns whether the controller collides downwards
	virtual bool controllerDoesCollideDown(ControllerInstance i) const = 0;
	// Returns whether the controller collides sidewards
	virtual bool controllerDoesCollideSides(ControllerInstance i) const = 0;

	// Creates joint
	virtual JointInstance jointCreate(ActorInstance a0, ActorInstance a1, const JointDesc& jointDesc) = 0;
	virtual void jointDestroy(JointInstance i) = 0;

	// Performs a raycast
	virtual void raycast(const Vector3& from, const Vector3& direction, float length, RaycastMode::Enum mode, Array<RaycastHit>& hits) = 0;
	// Returns the gravity
	virtual Vector3 getGravity() const = 0;
	// Sets the gravity
	virtual void setGravity(const Vector3& g) = 0;
	virtual void updateActorWorldPoses(const UnitId* begin, const UnitId* end, const Matrix4x4* beginWorld) = 0;
	// Updates the physics simulation
	virtual void update(float dt) = 0;
	virtual EventStream& getEventStream() = 0;
	virtual void debugDraw() = 0;
	virtual void enableDebugDrawing(bool enable) = 0;
};

namespace PhysicsWorldFn
{
	PhysicsWorld* create(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager, DebugLine& debugLine);
	void destroy(Allocator& a, PhysicsWorld* physicsWorld);
} // namespace PhysicsWorldFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka