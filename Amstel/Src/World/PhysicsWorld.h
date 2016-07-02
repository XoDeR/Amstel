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

	virtual ColliderInstance createCollider(UnitId id, const ColliderDesc* colliderDesc) = 0;
	virtual ColliderInstance getFirstCollider(UnitId id) = 0;
	virtual ColliderInstance getNextCollider(ColliderInstance i) = 0;

	virtual ActorInstance createActor(UnitId id, const ActorResource* actorResource, const Matrix4x4& transformMatrix) = 0;
	virtual void destroyActor(ActorInstance i) = 0;
	virtual ActorInstance getActor(UnitId id) = 0;
	// Returns the world position of the actor
	virtual Vector3 getActorWorldPosition(ActorInstance i) const = 0;
	// Returns the world rotation of the actor
	virtual Quaternion getActorWorldRotation(ActorInstance i) const = 0;
	// Returns the world pose of the actor
	virtual Matrix4x4 getActorWorldPose(ActorInstance i) const = 0;
	// Teleports the actor to the given world position
	virtual void teleportActorWorldPosition(ActorInstance i, const Vector3& p) = 0;
	// Teleports the actor to the given world rotation
	virtual void teleportActorWorldRotation(ActorInstance i, const Quaternion& r) = 0;
	// Teleports the actor to the given world pose
	virtual void teleportActorWorldPose(ActorInstance i, const Matrix4x4& m) = 0;
	// Returns the center of mass of the actor
	virtual Vector3 getActorCenterOfMass(ActorInstance i) const = 0;
	// Enables gravity for the actor
	virtual void enableActorGravity(ActorInstance i) = 0;
	// Disables gravity for the actor
	virtual void disableActorGravity(ActorInstance i) = 0;
	// Enables collision detection for the actor
	virtual void enableActorCollision(ActorInstance i) = 0;
	// Disables collision detection for the actor
	virtual void disableActorCollision(ActorInstance i) = 0;
	// Sets the collision filter of the actor
	virtual void setActorCollisionFilter(ActorInstance i, StringId32 filter) = 0;
	// Sets whether the actor is kinematic or not
	// This call has no effect on static actors
	virtual void setActorKinematic(ActorInstance i, bool kinematic) = 0;
	// Moves the actor to <position>
	// This call only affects nonkinematic actors
	virtual void moveActor(ActorInstance i, const Vector3& position) = 0;
	virtual bool getIsStatic(ActorInstance i) const = 0;
	virtual bool getIsDynamic(ActorInstance i) const = 0;
	// Returns whether the actor is kinematic (keyframed)
	virtual bool getIsKinematic(ActorInstance i) const = 0;
	// Returns whether the actor is nonkinematic (dynamic and not kinematic)
	virtual bool getIsNonKinematic(ActorInstance i) const = 0;
	// Returns the linear damping of the actor
	virtual float getActorLinearDamping(ActorInstance i) const = 0;
	// Sets the linear damping of the actor
	virtual void setActorLinearDamping(ActorInstance i, float rate) = 0;
	// Returns the angular damping of the actor
	virtual float getActorAngularDamping(ActorInstance i) const = 0;
	// Sets the angular damping of the actor
	virtual void setActorAngularDamping(ActorInstance i, float rate) = 0;
	// Returns the linear velocity of the actor
	virtual Vector3 getActorLinearVelocity(ActorInstance i) const = 0;
	// Sets the linear velocity of the actor
	// This call only affects nonkinematic actors
	virtual void setActorLinearVelocity(ActorInstance i, const Vector3& velocity) = 0;
	// Returns the angular velocity of the actor
	virtual Vector3 getActorAngularVelocity(ActorInstance i) const = 0;
	// Sets the angular velocity of the actor
	// This call only affects nonkinematic actors
	virtual void setActorAngularVelocity(ActorInstance i, const Vector3& velocity) = 0;
	// Adds a linear impulse (acting along the center of mass) to the actor
	// This call only affects nonkinematic actors
	virtual void addActorImpulse(ActorInstance i, const Vector3& impulse) = 0;
	// Adds a linear impulse (acting along the world <position>) to the actor
	// This call only affects nonkinematic actors
	virtual void addActorImpulseAt(ActorInstance i, const Vector3& impulse, const Vector3& position) = 0;
	// Adds a torque impulse to the actor
	virtual void addActorTorqueImpulse(ActorInstance i, const Vector3& impulse) = 0;
	// Pushes the actor as if it was hit by a point object with the given <mass>
	// travelling at the given <velocity>
	// This call only affects nonkinematic actors
	virtual void pushActor(ActorInstance i, const Vector3& velocity, float mass) = 0;
	// Like push() but applies the force at the world position <position>
	// This call only affects nonkinematic actors
	virtual void pushActorAt(ActorInstance i, const Vector3& velocity, float mass, const Vector3& position) = 0;
	// Returns whether the actor is sleeping
	virtual bool getIsSleeping(ActorInstance i) = 0;
	// Wakes the actor up
	virtual void wakeUp(ActorInstance i) = 0;

	virtual ControllerInstance createController(UnitId id, const ControllerDesc& cd, const Matrix4x4& transformMatrix) = 0;
	virtual void destroyController(ControllerInstance id) = 0;
	virtual ControllerInstance getController(UnitId id) = 0;
	// Returns the position of the controller
	virtual Vector3 getPosition(ControllerInstance i) const = 0;
	// Moves the controller to <position>
	virtual void moveController(ControllerInstance i, const Vector3& position) = 0;
	// Sets the contoller height
	virtual void setHeight(ControllerInstance i, float height) = 0;
	// Returns whether the contoller collides upwards
	virtual bool doesCollideUp(ControllerInstance i) const = 0;
	// Returns whether the controller collides downwards
	virtual bool doesCollideDown(ControllerInstance i) const = 0;
	// Returns whether the controller collides sidewards
	virtual bool doesCollideSides(ControllerInstance i) const = 0;

	// Creates joint
	virtual JointInstance createJoint(ActorInstance a0, ActorInstance a1, const JointDesc& jointDesc) = 0;
	virtual void destroyJoint(JointInstance i) = 0;
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
	virtual void drawDebug() = 0;
	virtual void enableDebugDrawing(bool enable) = 0;
};

namespace PhysicsWorldFn
{
	PhysicsWorld* create(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager, DebugLine& debugLine);
	void destroy(Allocator& a, PhysicsWorld* physicsWorld);
} // namespace PhysicsWorldFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka