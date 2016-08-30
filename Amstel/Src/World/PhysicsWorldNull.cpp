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

	virtual ColliderInstance colliderCreate(UnitId /*id*/, const ColliderDesc* /*colliderDesc*/) override
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual void colliderDestroy(ColliderInstance colliderInstance) override
	{
	}

	virtual ColliderInstance colliderGetFirst(UnitId /*id*/) override
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual ColliderInstance colliderGetNext(ColliderInstance /*i*/) override
	{
		return makeColliderInstance(UINT32_MAX);
	}

	virtual ActorInstance actorCreate(UnitId /*id*/, const ActorResource* /*actorResource*/, const Matrix4x4& /*transformMatrix*/)
	{
		return makeActorInstance(UINT32_MAX);
	}

	virtual void actorDestroy(ActorInstance /*i*/)
	{
	}

	virtual ActorInstance actorGet(UnitId /*id*/)
	{
		return makeActorInstance(UINT32_MAX);
	}

	virtual Vector3 actorGetWorldPosition(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual Quaternion actorGetWorldRotation(ActorInstance /*i*/) const
	{
		return QUATERNION_IDENTITY;
	}

	virtual Matrix4x4 actorGetWorldPose(ActorInstance /*i*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	virtual void actorTeleportWorldPosition(ActorInstance /*i*/, const Vector3& /*p*/)
	{
	}

	virtual void actorTeleportWorldRotation(ActorInstance /*i*/, const Quaternion& /*r*/)
	{
	}

	virtual void actorTeleportWorldPose(ActorInstance /*i*/, const Matrix4x4& /*m*/)
	{
	}

	virtual Vector3 actorGetCenterOfMass(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actorEnableGravity(ActorInstance /*i*/)
	{
	}

	virtual void actorDisableGravity(ActorInstance /*i*/)
	{
	}

	virtual void actorEnableCollision(ActorInstance /*i*/)
	{
	}

	virtual void actorDisableCollision(ActorInstance /*i*/)
	{
	}

	virtual void actorSetCollisionFilter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
	}

	virtual void actorSetKinematic(ActorInstance /*i*/, bool /*kinematic*/)
	{
	}

	virtual void actorMove(ActorInstance /*i*/, const Vector3& /*position*/)
	{
	}

	virtual bool actorGetIsStatic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actorGetIsDynamic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actorGetIsKinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actorGetIsNonKinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual float actorGetLinearDamping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void actorSetLinearDamping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual float actorGetAngularDamping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void actorSetAngularDamping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual Vector3 actorGetLinearVelocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actorSetLinearVelocity(ActorInstance /*i*/, const Vector3& /*velocity*/)
	{
	}

	virtual Vector3 actorGetAngularVelocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actorSetAngularVelocity(ActorInstance /*i*/, const Vector3& /*velocity*/)
	{
	}

	virtual void actorAddImpulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void actorAddImpulseAt(ActorInstance /*i*/, const Vector3& /*impulse*/, const Vector3& /*position*/)
	{
	}

	virtual void actorAddTorqueImpulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void actorPush(ActorInstance /*i*/, const Vector3& /*velocity*/, float /*mass*/)
	{
	}

	virtual void actorPushAt(ActorInstance /*i*/, const Vector3& /*velocity*/, float /*mass*/, const Vector3& /*position*/)
	{
	}

	virtual bool actorGetIsSleeping(ActorInstance /*i*/)
	{
		return false;
	}

	virtual void actorWakeUp(ActorInstance /*i*/)
	{
	}

	virtual ControllerInstance controllerCreate(UnitId /*id*/, const ControllerDesc& /*controllerDesc*/, const Matrix4x4& /*transformMatrix*/)
	{
		return makeControllerInstance(UINT32_MAX);
	}

	virtual void controllerDestroy(ControllerInstance /*id*/)
	{
	}

	virtual ControllerInstance controllerGet(UnitId /*id*/)
	{
		return makeControllerInstance(UINT32_MAX);
	}

	virtual Vector3 controllerGetPosition(ControllerInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void controllerMove(ControllerInstance /*i*/, const Vector3& /*position*/)
	{
	}

	virtual void controllerSetHeight(ControllerInstance /*i*/, float /*height*/)
	{
	}

	virtual bool controllerDoesCollideUp(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool controllerDoesCollideDown(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool controllerDoesCollideSides(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual JointInstance jointCreate(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jointDesc*/)
	{
		return makeJointInstance(UINT32_MAX);
	}

	virtual void jointDestroy(JointInstance /*i*/)
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

	virtual void debugDraw() override
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