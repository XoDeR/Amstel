// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PHYSICS_NULL

#include "World/PhysicsWorld.h"

namespace Rio
{

namespace PhysicsGlobalFn
{
	void init()
	{
	}

	void shutdown()
	{
	}
} // namespace PhysicsGlobalFn

class PhysicsWorldNull : public PhysicsWorld
{
public:
	PhysicsWorldNull(Allocator& a)
		: eventStream(a)
	{
	}

	virtual ~PhysicsWorldNull()
	{
	}

	virtual ColliderInstance createCollider(UnitId /*id*/, const ColliderDesc* /*colliderDesc*/)
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual ColliderInstance getFirstCollider(UnitId /*id*/)
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual ColliderInstance getNextCollider(ColliderInstance /*i*/)
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual ActorInstance createActor(UnitId /*id*/, const ActorResource* /*actorResource*/, const Matrix4x4& /*transformMatrix*/)
	{
		return makeActorInstance(UINT32_MAX);
	}

	virtual void destroyActor(ActorInstance /*i*/)
	{
	}

	virtual ActorInstance getActor(UnitId /*id*/)
	{
		return makeActorInstance(UINT32_MAX);
	}

	virtual Vector3 getActorWorldPosition(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual Quaternion getActorWorldRotation(ActorInstance /*i*/) const
	{
		return QUATERNION_IDENTITY;
	}

	virtual Matrix4x4 getActorWorldPose(ActorInstance /*i*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	virtual void teleportActorWorldPosition(ActorInstance /*i*/, const Vector3& /*p*/)
	{
	}

	virtual void teleportActorWorldRotation(ActorInstance /*i*/, const Quaternion& /*r*/)
	{
	}

	virtual void teleportActorWorldPose(ActorInstance /*i*/, const Matrix4x4& /*m*/)
	{
	}

	virtual Vector3 getActorCenterOfMass(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void enableActorGravity(ActorInstance /*i*/)
	{
	}

	virtual void disableActorGravity(ActorInstance /*i*/)
	{
	}

	virtual void enableActorCollision(ActorInstance /*i*/)
	{
	}

	virtual void disableActorCollision(ActorInstance /*i*/)
	{
	}

	virtual void setActorCollisionFilter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
	}

	virtual void setActorKinematic(ActorInstance /*i*/, bool /*kinematic*/)
	{
	}

	virtual void moveActor(ActorInstance /*i*/, const Vector3& /*position*/)
	{
	}

	virtual bool getIsStatic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool getIsDynamic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool getIsKinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool getIsNonKinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual float getActorLinearDamping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void setActorLinearDamping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual float getActorAngularDamping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void setActorAngularDamping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual Vector3 getActorLinearVelocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void setActorLinearVelocity(ActorInstance /*i*/, const Vector3& /*velocity*/)
	{
	}

	virtual Vector3 getActorAngularVelocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void setActorAngularVelocity(ActorInstance /*i*/, const Vector3& /*velocity*/)
	{
	}

	virtual void addActorImpulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void addActorImpulseAt(ActorInstance /*i*/, const Vector3& /*impulse*/, const Vector3& /*position*/)
	{
	}

	virtual void addActorTorqueImpulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void pushActor(ActorInstance /*i*/, const Vector3& /*velocity*/, float /*mass*/)
	{
	}

	virtual void pushActorAt(ActorInstance /*i*/, const Vector3& /*velocity*/, float /*mass*/, const Vector3& /*position*/)
	{
	}

	virtual bool getIsSleeping(ActorInstance /*i*/)
	{
		return false;
	}

	virtual void wakeUp(ActorInstance /*i*/)
	{
	}

	virtual ControllerInstance createController(UnitId /*id*/, const ControllerDesc& /*cd*/, const Matrix4x4& /*transformMatrix*/)
	{
		return makeControllerInstance(UINT32_MAX);
	}

	virtual void destroyController(ControllerInstance /*id*/)
	{
	}

	virtual ControllerInstance getController(UnitId /*id*/)
	{
		return makeControllerInstance(UINT32_MAX);
	}

	virtual Vector3 getPosition(ControllerInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void moveController(ControllerInstance /*i*/, const Vector3& /*position*/)
	{
	}

	virtual void setHeight(ControllerInstance /*i*/, float /*height*/)
	{
	}

	virtual bool doesCollideUp(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool doesCollideDown(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool doesCollideSides(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual JointInstance createJoint(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jointDesc*/)
	{
		return makeJointInstance(UINT32_MAX);
	}

	virtual void destroyJoint(JointInstance /*i*/)
	{
	}

	virtual void raycast(const Vector3& /*from*/, const Vector3& /*direction*/, float /*length*/, RaycastMode::Enum /*mode*/, Array<RaycastHit>& /*hits*/)
	{
	}

	virtual Vector3 getGravity() const
	{
		return VECTOR3_ZERO;
	}

	virtual void setGravity(const Vector3& /*g*/)
	{
	}

	virtual void updateActorWorldPoses(const UnitId* /*begin*/, const UnitId* /*end*/, const Matrix4x4* /*beginWorld*/)
	{
	}

	virtual void update(float /*dt*/)
	{
	}

	virtual EventStream& getEventStream()
	{
		return eventStream;
	}

	virtual void drawDebug()
	{
	}

	virtual void enableDebugDrawing(bool /*enable*/)
	{
	}

private:
	ColliderInstance makeColliderInstance(uint32_t i) { ColliderInstance colliderInstance = { i }; return colliderInstance; }
	ActorInstance makeActorInstance(uint32_t i) { ActorInstance actorInstance = { i }; return actorInstance; }
	ControllerInstance makeControllerInstance(uint32_t i) { ControllerInstance controllerInstance = { i }; return controllerInstance; }
	JointInstance makeJointInstance(uint32_t i) { JointInstance jointInstance = { i }; return jointInstance; }

	EventStream eventStream;
};

PhysicsWorld* PhysicsWorldFn::create(Allocator& a, ResourceManager& /*resourceManager*/, UnitManager& /*unitManager*/, DebugLine& /*debugLine*/)
{
	return RIO_NEW(a, PhysicsWorldNull)(a);
}

void PhysicsWorldFn::destroy(Allocator& a, PhysicsWorld* physicsWorld)
{
	RIO_DELETE(a, physicsWorld);
}

} // namespace Rio

#endif // RIO_PHYSICS_NULL
// Copyright (c) 2016 Volodymyr Syvochka