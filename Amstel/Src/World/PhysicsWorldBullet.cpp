#include "Config.h"

#if 1//RIO_PHYSICS_BULLET

#include "Core/Containers/Array.h"
#include "Core/Math/Color4.h"
#include "Core/Containers/HashMap.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"
#include "Device/Log.h"
#include "Resource/ResourceManager.h"
#include "Resource/PhysicsResource.h"
#include "World/Physics.h"
#include "World/PhysicsWorld.h"
#include "World/DebugLine.h"
#include "World/UnitManager.h"

#include "btBoxShape.h"
#include "btBvhTriangleMeshShape.h"
#include "btCapsuleShape.h"
#include "btCollisionObject.h"
#include "btCompoundShape.h"
#include "btConvexHullShape.h"
#include "btConvexTriangleMeshShape.h"
#include "btDbvtBroadphase.h"
#include "btDefaultCollisionConfiguration.h"
#include "btDefaultMotionState.h"
#include "btDiscreteDynamicsWorld.h"
#include "btFixedConstraint.h"
#include "btGhostObject.h"
#include "btHeightfieldTerrainShape.h"
#include "btHingeConstraint.h"
#include "btIDebugDraw.h"
#include "btKinematicCharacterController.h"
#include "btPoint2PointConstraint.h"
#include "btRigidBody.h"
#include "btSequentialImpulseConstraintSolver.h"
#include "btSliderConstraint.h"
#include "btSphereShape.h"
#include "btStaticPlaneShape.h"
#include "btTriangleMesh.h"

namespace Rio
{

namespace PhysicsGlobalFn
{
	static btDefaultCollisionConfiguration* defaultCollisionConfiguration;
	static btCollisionDispatcher* collisionDispatcher;
	static btBroadphaseInterface* broadphaseInterface;
	static btSequentialImpulseConstraintSolver* sequentialImpulseConstraintSolver;

	void init(Allocator& a)
	{
		defaultCollisionConfiguration = RIO_NEW(a, btDefaultCollisionConfiguration);
		collisionDispatcher = RIO_NEW(a, btCollisionDispatcher)(defaultCollisionConfiguration);
		broadphaseInterface = RIO_NEW(a, btDbvtBroadphase);
		sequentialImpulseConstraintSolver = RIO_NEW(a, btSequentialImpulseConstraintSolver);
	}

	void shutdown(Allocator& a)
	{
		RIO_DELETE(a, sequentialImpulseConstraintSolver);
		RIO_DELETE(a, broadphaseInterface);
		RIO_DELETE(a, collisionDispatcher);
		RIO_DELETE(a, defaultCollisionConfiguration);
	}
	
} // namespace PhysicsGlobalFn

static btVector3 getBtVector3(const Vector3& v)
{
	return btVector3(v.x, v.y, v.z);
}

static btQuaternion getBtQuaternion(const Quaternion& q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

static btTransform getBtTransform(const Matrix4x4& m)
{
	btMatrix3x3 basis(m.x.x, m.y.x, m.z.x
		, m.x.y, m.y.y, m.z.y
		, m.x.z, m.y.z, m.z.z
		);
	btVector3 position(m.t.x, m.t.y, m.t.z);
	return btTransform(basis, position);
}

static Vector3 getVector3(const btVector3& v)
{
	return createVector3(v.x(), v.y(), v.z());
}

static Quaternion getQuaternion(const btQuaternion& q)
{
	return createQuaternion(q.x(), q.y(), q.z(), q.w());
}

class DebugDrawer : public btIDebugDraw
{
public:
	DebugDrawer(DebugLine& debugLine)
		: debugLine(&debugLine)
	{
	}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& /*color*/)
	{
		const Vector3 start = getVector3(from);
		const Vector3 end = getVector3(to);
		debugLine->addLine(start, end, COLOR4_ORANGE);
	}

	void drawContactPoint(const btVector3& pointOnB, const btVector3& /*normalOnB*/, btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
	{
		const Vector3 from = getVector3(pointOnB);
		debugLine->addSphere(from, 0.1f, COLOR4_WHITE);
	}

	void reportErrorWarning(const char* warningString)
	{
		RIO_LOGW(warningString);
	}

	void draw3dText(const btVector3& /*location*/, const char* /*textString*/)
	{
	}

	void setDebugMode(int /*debugMode*/)
	{
	}

	int getDebugMode() const
	{
		return DBG_DrawWireframe
			| DBG_DrawConstraints
			| DBG_DrawConstraintLimits
			| DBG_FastWireframe
			;
	}

public:
	DebugLine* debugLine;
};

class OverlapFilterCallback : public btOverlapFilterCallback
{
	bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
	{
		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
		return collides;
	}
};

class BulletWorld : public PhysicsWorld
{
public:
	BulletWorld(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager, DebugLine& debugLine)
		: allocator(&a)
		, unitManager(&unitManager)
		, colliderMap(a)
		, actorMap(a)
		, controllerMap(a)
		, colliderList(a)
		, actorList(a)
		, controllerList(a)
		, jointList(a)
		, debugDrawer(debugLine)
		, eventStream(a)
	{
		discreteDynamicsWorld = RIO_NEW(*allocator, btDiscreteDynamicsWorld)(PhysicsGlobalFn::collisionDispatcher
			, PhysicsGlobalFn::broadphaseInterface
			, PhysicsGlobalFn::sequentialImpulseConstraintSolver
			, PhysicsGlobalFn::defaultCollisionConfiguration
			);

		discreteDynamicsWorld->getCollisionWorld()->setDebugDrawer(&debugDrawer);
		discreteDynamicsWorld->setInternalTickCallback(tickCallbackWrapper, this);
		discreteDynamicsWorld->getPairCache()->setOverlapFilterCallback(&overlapFilterCallback);

		physicsConfigResource = (const PhysicsConfigResource*)resourceManager.get(RESOURCE_TYPE_PHYSICS_CONFIG, StringId64("global"));

		unitManager.registerDestroyFunction(BulletWorld::unitDestroyedCallback, this);
	}

	~BulletWorld()
	{
		unitManager->unregisterDestroyFunction(this);

		for (uint32_t i = 0; i < ArrayFn::getCount(actorList); ++i)
		{
			btRigidBody* rigidBody = actorList[i].actor;

			discreteDynamicsWorld->removeRigidBody(rigidBody);
			RIO_DELETE(*allocator, rigidBody->getMotionState());
			RIO_DELETE(*allocator, rigidBody->getCollisionShape());
			RIO_DELETE(*allocator, rigidBody);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(colliderList); ++i)
		{
			RIO_DELETE(*allocator, colliderList[i].vertexArray);
			RIO_DELETE(*allocator, colliderList[i].shape);
		}

		RIO_DELETE(*allocator, discreteDynamicsWorld);
	}

	virtual ColliderInstance colliderCreate(UnitId id, const ColliderDesc* colliderDesc) override
	{
		btTriangleIndexVertexArray* vertexArray = nullptr;
		btCollisionShape* childShape = nullptr;

		switch(colliderDesc->type)
		{
			case ColliderType::SPHERE:
				childShape = RIO_NEW(*allocator, btSphereShape)(colliderDesc->sphere.radius);
				break;
			case ColliderType::CAPSULE:
				childShape = RIO_NEW(*allocator, btCapsuleShape)(colliderDesc->capsule.radius, colliderDesc->capsule.height);
				break;
			case ColliderType::BOX:
				childShape = RIO_NEW(*allocator, btBoxShape)(getBtVector3(colliderDesc->box.halfSize));
				break;
			case ColliderType::CONVEX_HULL:
			{
				const char* data = (char*)&colliderDesc[1];
				const uint32_t count = *(uint32_t*)data;
				const btScalar* pointList = (btScalar*)(data + sizeof(uint32_t));

				childShape = RIO_NEW(*allocator, btConvexHullShape)(pointList, (int)count, sizeof(Vector3));
			}
			break;
			case ColliderType::MESH:
			{
				const char* data = (char*)&colliderDesc[1];
				const uint32_t pointsCount = *(uint32_t*)data;
				const char* points = data + sizeof(uint32_t);
				const uint32_t indicesCount = *(uint32_t*)(points + pointsCount *sizeof(Vector3));
				const char* indices = points + sizeof(uint32_t) + pointsCount *sizeof(Vector3);

				btIndexedMesh part;
				part.m_vertexBase = (const unsigned char*)points;
				part.m_vertexStride = sizeof(Vector3);
				part.m_numVertices = pointsCount;
				part.m_triangleIndexBase = (const unsigned char*)indices;
				part.m_triangleIndexStride = sizeof(uint16_t)*3;
				part.m_numTriangles = indicesCount / 3;
				part.m_indexType = PHY_SHORT;

				vertexArray = RIO_NEW(*allocator, btTriangleIndexVertexArray)();
				vertexArray->addIndexedMesh(part, PHY_SHORT);

				const btVector3 aabbMin(-1000.0f,-1000.0f,-1000.0f);
				const btVector3 aabbMax(1000.0f,1000.0f,1000.0f);
				childShape = RIO_NEW(*allocator, btBvhTriangleMeshShape)(vertexArray, false, aabbMin, aabbMax);
			}
			break;
			case ColliderType::HEIGHTFIELD:
			{
				RIO_FATAL("Not implemented yet");
			}
			break;
			default:
			{
				RIO_FATAL("Bad shape");
			}
			break;
		}

		const uint32_t last = ArrayFn::getCount(colliderList);

		ColliderInstanceData colliderInstanceData;
		colliderInstanceData.unitId = id;
		colliderInstanceData.localTransformMatrix = colliderDesc->localTransformMatrix;
		colliderInstanceData.vertexArray = vertexArray;
		colliderInstanceData.shape = childShape;
		colliderInstanceData.next.i = UINT32_MAX;

		ColliderInstance colliderInstance = colliderGetFirst(id);
		while (getIsValid(colliderInstance) && getIsValid(colliderGetNext(colliderInstance)))
		{
			colliderInstance = colliderGetNext(colliderInstance);
		}

		if (getIsValid(colliderInstance))
		{
			colliderList[colliderInstance.i].next.i = last;
		}
		else
		{
			HashMapFn::set(colliderMap, id, last);
		}

		ArrayFn::pushBack(colliderList, colliderInstanceData);
		return makeColliderInstance(last);
	}

	// private implementation
	// auxiliary function
	void colliderRemoveNode(ColliderInstance first, ColliderInstance colliderInstance)
	{
		RIO_ASSERT(first.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");
		RIO_ASSERT(colliderInstance.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");

		const UnitId unitId = this->colliderList[first.i].unitId;

		if (colliderInstance.i == first.i)
		{
			if (!getIsValid(colliderGetNext(colliderInstance)))
			{
				HashMapFn::remove(colliderMap, unitId);
			}
			else
			{
				HashMapFn::set(colliderMap, unitId, colliderGetNext(colliderInstance).i);
			}
		}
		else
		{
			ColliderInstance previousColliderInstance = colliderGetPrevious(colliderInstance);
			this->colliderList[previousColliderInstance.i].next = colliderGetNext(colliderInstance);
		}
	}

	void colliderSwapNode(ColliderInstance a, ColliderInstance b)
	{
		RIO_ASSERT(a.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");
		RIO_ASSERT(b.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");

		const UnitId unitId = this->colliderList[a.i].unitId;
		const ColliderInstance firstColliderInstance = colliderGetFirst(unitId);

		if (a.i == firstColliderInstance.i)
		{
			HashMapFn::set(colliderMap, unitId, b.i);
		}
		else
		{
			const ColliderInstance previousA = colliderGetPrevious(a);
			RIO_ENSURE(previousA.i != a.i);
			this->colliderList[previousA.i].next = b;
		}
	}

	ColliderInstance colliderGetFirst(UnitId id)
	{
		return makeColliderInstance(HashMapFn::get(colliderMap, id, UINT32_MAX));
	}

	ColliderInstance colliderGetNext(ColliderInstance i)
	{
		return colliderList[i.i].next;
	}

	ColliderInstance colliderGetPrevious(ColliderInstance colliderInstance)
	{
		RIO_ASSERT(colliderInstance.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");

		const UnitId unitId = this->colliderList[colliderInstance.i].unitId;

		ColliderInstance currentColliderInstance = colliderGetFirst(unitId);
		ColliderInstance previousColliderInstance = { UINT32_MAX };

		while (currentColliderInstance.i != colliderInstance.i)
		{
			previousColliderInstance = currentColliderInstance;
			currentColliderInstance = colliderGetNext(currentColliderInstance);
		}

		return previousColliderInstance;
	}

	virtual void colliderDestroy(ColliderInstance colliderInstance) override
	{
		RIO_ASSERT(colliderInstance.i < ArrayFn::getCount(this->colliderList), "Index out of bounds");

		const uint32_t last = ArrayFn::getCount(this->colliderList) - 1;
		const UnitId unitId = this->colliderList[colliderInstance.i].unitId;
		const ColliderInstance firstColliderInstance = colliderGetFirst(unitId);
		const ColliderInstance lastColliderInstance = makeColliderInstance(last);

		colliderSwapNode(lastColliderInstance, colliderInstance);
		colliderRemoveNode(firstColliderInstance, colliderInstance);

		RIO_DELETE(*(this->allocator), this->colliderList[colliderInstance.i].vertexArray);
		RIO_DELETE(*(this->allocator), this->colliderList[colliderInstance.i].shape);

		this->colliderList[colliderInstance.i] = colliderList[last];

		ArrayFn::popBack(this->colliderList);
	}

	ActorInstance actorCreate(UnitId unitId, const ActorResource* actorResource, const Matrix4x4& transformMatrix)
	{
		const PhysicsConfigActor* actorClass = PhysicsConfigResourceFn::getPhysicsConfigActor(physicsConfigResource, actorResource->actorClass);

		const bool isKinematic = (actorClass->flags & PhysicsConfigActor::KINEMATIC) != 0;
		const bool isDynamic = (actorClass->flags & PhysicsConfigActor::DYNAMIC) != 0;
		const bool isStatic = !isKinematic && !isDynamic;
		const float mass = isDynamic ? actorResource->mass : 0.0f;

		// Create compound shape
		btCompoundShape* shape = RIO_NEW(*allocator, btCompoundShape)(true);

		ColliderInstance colliderInstance = colliderGetFirst(unitId);
		while (getIsValid(colliderInstance) == true)
		{
			shape->addChildShape(btTransform::getIdentity(), colliderList[colliderInstance.i].shape);
			colliderInstance = colliderGetNext(colliderInstance);
		}

		// Create motion state
		btDefaultMotionState* defaultMotionState = RIO_NEW(*allocator, btDefaultMotionState)(getBtTransform(transformMatrix));

		// If dynamic, calculate inertia
		btVector3 inertia;
		if (mass != 0.0f) // Actor is dynamic iff mass != 0
		{
			shape->calculateLocalInertia(mass, inertia);
		}

		btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfo(mass, defaultMotionState, shape, inertia);
		rigidBodyConstructionInfo.m_linearDamping = actorClass->linearDamping;
		rigidBodyConstructionInfo.m_angularDamping = actorClass->angularDamping;
		rigidBodyConstructionInfo.m_restitution = 0.81f; // TODO
		rigidBodyConstructionInfo.m_friction = 0.8f; // TODO
		rigidBodyConstructionInfo.m_rollingFriction = 0.5f; // TODO
		rigidBodyConstructionInfo.m_linearSleepingThreshold = 0.01f; // TODO
		rigidBodyConstructionInfo.m_angularSleepingThreshold = 0.01f; // TODO

		// Create rigid body
		btRigidBody* actor = RIO_NEW(*allocator, btRigidBody)(rigidBodyConstructionInfo);

		int collisionFlags = actor->getCollisionFlags();
		collisionFlags |= isKinematic ? btCollisionObject::CF_KINEMATIC_OBJECT : 0;
		collisionFlags |= isStatic ? btCollisionObject::CF_STATIC_OBJECT : 0;
		actor->setCollisionFlags(collisionFlags);

		actor->setLinearFactor(btVector3(
			(actorResource->flags & ActorFlags::LOCK_TRANSLATION_X) ? 0.0f : 1.0f,
			(actorResource->flags & ActorFlags::LOCK_TRANSLATION_Y) ? 0.0f : 1.0f,
			(actorResource->flags & ActorFlags::LOCK_TRANSLATION_Z) ? 0.0f : 1.0f)
		);
		actor->setAngularFactor(btVector3(
			(actorResource->flags & ActorFlags::LOCK_ROTATION_X) ? 0.0f : 1.0f,
			(actorResource->flags & ActorFlags::LOCK_ROTATION_Y) ? 0.0f : 1.0f,
			(actorResource->flags & ActorFlags::LOCK_ROTATION_Z) ? 0.0f : 1.0f)
		);

		const uint32_t last = ArrayFn::getCount(actorList);

		actor->setUserPointer((void*)(uintptr_t)last);

		// Set collision filters
		const uint32_t me = PhysicsConfigResourceFn::getPhysicsCollisionFilter(physicsConfigResource, actorResource->collisionFilter)->me;
		const uint32_t mask = PhysicsConfigResourceFn::getPhysicsCollisionFilter(physicsConfigResource, actorResource->collisionFilter)->mask;

		discreteDynamicsWorld->addRigidBody(actor, me, mask);

		ActorInstanceData actorInstanceData;
		actorInstanceData.unitId = unitId;
		actorInstanceData.actor = actor;

		ArrayFn::pushBack(actorList, actorInstanceData);
		HashMapFn::set(actorMap, unitId, last);

		return makeActorInstance(last);
	}

	void actorDestroy(ActorInstance actorInstance)
	{
		const uint32_t lastActorIndex = ArrayFn::getCount(actorList) - 1;
		const UnitId unitId = actorList[actorInstance.i].unitId;
		const UnitId lastUnitId = actorList[lastActorIndex].unitId;

		discreteDynamicsWorld->removeRigidBody(actorList[actorInstance.i].actor);
		RIO_DELETE(*allocator, actorList[actorInstance.i].actor->getMotionState());
		RIO_DELETE(*allocator, actorList[actorInstance.i].actor->getCollisionShape());
		RIO_DELETE(*allocator, actorList[actorInstance.i].actor);

		actorList[actorInstance.i] = actorList[lastActorIndex];
		actorList[actorInstance.i].actor->setUserPointer((void*)(uintptr_t)actorInstance.i);

		ArrayFn::popBack(actorList);

		HashMapFn::set(actorMap, lastUnitId, actorInstance.i);
		HashMapFn::remove(actorMap, unitId);
	}

	ActorInstance actorGet(UnitId id)
	{
		return makeActorInstance(HashMapFn::get(actorMap, id, UINT32_MAX));
	}

	Vector3 actorGetWorldPosition(ActorInstance i) const
	{
		btTransform pose;
		actorList[i.i].actor->getMotionState()->getWorldTransform(pose);
		const btVector3 position = pose.getOrigin();
		return getVector3(position);
	}

	Quaternion actorGetWorldRotation(ActorInstance i) const
	{
		btTransform pose;
		actorList[i.i].actor->getMotionState()->getWorldTransform(pose);
		return getQuaternion(pose.getRotation());
	}

	Matrix4x4 actorGetWorldPose(ActorInstance i) const
	{
		btTransform pose;
		actorList[i.i].actor->getMotionState()->getWorldTransform(pose);
		const btQuaternion rotation = pose.getRotation();
		const btVector3 position = pose.getOrigin();
		return createMatrix4x4(getQuaternion(rotation), getVector3(position));
	}

	void actorTeleportWorldPosition(ActorInstance i, const Vector3& position)
	{
		btTransform pose = actorList[i.i].actor->getCenterOfMassTransform();
		pose.setOrigin(getBtVector3(position));
		actorList[i.i].actor->setCenterOfMassTransform(pose);
	}

	void actorTeleportWorldRotation(ActorInstance i, const Quaternion& rotation)
	{
		btTransform pose = actorList[i.i].actor->getCenterOfMassTransform();
		pose.setRotation(getBtQuaternion(rotation));
		actorList[i.i].actor->setCenterOfMassTransform(pose);
	}

	void actorTeleportWorldPose(ActorInstance i, const Matrix4x4& m)
	{
		const Quaternion rotation = getRotationAsQuaternion(m);
		const Vector3 position = getTranslation(m);

		btTransform pose = actorList[i.i].actor->getCenterOfMassTransform();
		pose.setRotation(getBtQuaternion(rotation));
		pose.setOrigin(getBtVector3(position));
		actorList[i.i].actor->setCenterOfMassTransform(pose);
	}

	Vector3 actorGetCenterOfMass(ActorInstance i) const
	{
		if (actorGetIsStatic(i))
		{
			return VECTOR3_ZERO;
		}

		const btVector3 centerOfMass = actorList[i.i].actor->getCenterOfMassTransform().getOrigin();
		return getVector3(centerOfMass);
	}

	void actorEnableGravity(ActorInstance i)
	{
		actorList[i.i].actor->setGravity(discreteDynamicsWorld->getGravity());
	}

	void actorDisableGravity(ActorInstance i)
	{
		actorList[i.i].actor->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	}

	void actorEnableCollision(ActorInstance /*i*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	void actorDisableCollision(ActorInstance /*i*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	void actorSetCollisionFilter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	void actorSetKinematic(ActorInstance i, bool kinematic)
	{
		if (kinematic == true)
		{
			actorList[i.i].actor->setCollisionFlags(actorList[i.i].actor->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		}
	}

	void actorMove(ActorInstance i, const Vector3& position)
	{
		if (!actorGetIsKinematic(i))
		{
			return;
		}

		actorList[i.i].actor->setLinearVelocity(getBtVector3(position));
	}

	bool actorGetIsStatic(ActorInstance i) const
	{
		return actorList[i.i].actor->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT;
	}

	bool actorGetIsDynamic(ActorInstance i) const
	{
		const int flags = actorList[i.i].actor->getCollisionFlags();
		return !(flags & btCollisionObject::CF_STATIC_OBJECT)
			&& !(flags & btCollisionObject::CF_KINEMATIC_OBJECT)
			;
	}

	bool actorGetIsKinematic(ActorInstance i) const
	{
		const int flags = actorList[i.i].actor->getCollisionFlags();
		return (flags & btCollisionObject::CF_KINEMATIC_OBJECT) != 0;
	}

	bool actorGetIsNonKinematic(ActorInstance i) const
	{
		return actorGetIsDynamic(i) && !actorGetIsKinematic(i);
	}

	float actorGetLinearDamping(ActorInstance i) const
	{
		return actorList[i.i].actor->getLinearDamping();
	}

	void actorSetLinearDamping(ActorInstance i, float rate)
	{
		actorList[i.i].actor->setDamping(rate, actorList[i.i].actor->getAngularDamping());
	}

	float actorGetAngularDamping(ActorInstance i) const
	{
		return actorList[i.i].actor->getAngularDamping();
	}

	void actorSetAngularDamping(ActorInstance i, float rate)
	{
		actorList[i.i].actor->setDamping(actorList[i.i].actor->getLinearDamping(), rate);
	}

	Vector3 actorGetLinearVelocity(ActorInstance i) const
	{
		btVector3 v = actorList[i.i].actor->getLinearVelocity();
		return getVector3(v);
	}

	void actorSetLinearVelocity(ActorInstance i, const Vector3& velocity)
	{
		actorList[i.i].actor->setLinearVelocity(getBtVector3(velocity));
	}

	Vector3 actorGetAngularVelocity(ActorInstance i) const
	{
		btVector3 v = actorList[i.i].actor->getAngularVelocity();
		return getVector3(v);
	}

	void actorSetAngularVelocity(ActorInstance i, const Vector3& velocity)
	{
		actorList[i.i].actor->setAngularVelocity(getBtVector3(velocity));
	}

	void actorAddImpulse(ActorInstance i, const Vector3& impulse)
	{
		actorList[i.i].actor->activate();
		actorList[i.i].actor->applyCentralImpulse(getBtVector3(impulse));
	}

	void actorAddImpulseAt(ActorInstance i, const Vector3& impulse, const Vector3& position)
	{
		actorList[i.i].actor->activate();
		actorList[i.i].actor->applyImpulse(getBtVector3(impulse), getBtVector3(position));
	}

	void actorAddTorqueImpulse(ActorInstance i, const Vector3& impulse)
	{
		actorList[i.i].actor->applyTorqueImpulse(getBtVector3(impulse));
	}

	void actorPush(ActorInstance i, const Vector3& velocity, float mass)
	{
		const Vector3 force = velocity * mass;
		actorList[i.i].actor->applyCentralForce(getBtVector3(force));
	}

	void actorPushAt(ActorInstance i, const Vector3& velocity, float mass, const Vector3& position)
	{
		const Vector3 force = velocity * mass;
		actorList[i.i].actor->applyForce(getBtVector3(force), getBtVector3(position));
	}

	bool actorGetIsSleeping(ActorInstance i)
	{
		return !actorList[i.i].actor->isActive();
	}

	void actorWakeUp(ActorInstance i)
	{
		actorList[i.i].actor->activate(true);
	}

	ControllerInstance controllerCreate(UnitId /*id*/,	const ControllerDesc& /*controllerDesc*/, const Matrix4x4& /*transformMatrix*/)
	{
		RIO_FATAL("Not implemented yet");
		return makeControllerInstance(UINT32_MAX);
	}

	void controllerDestroy(ControllerInstance /*i*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	ControllerInstance controllerGet(UnitId unitId)
	{
		return makeControllerInstance(HashMapFn::get(controllerMap, unitId, UINT32_MAX));
	}

	void controllerMove(ControllerInstance i, const Vector3& direction)
	{
		controllerList[i.i].kinematicCharacterController->setWalkDirection(getBtVector3(direction));
	}

	void controllerSetHeight(ControllerInstance /*i*/, float /*height*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	Vector3 controllerGetPosition(ControllerInstance /*i*/) const
	{
		RIO_FATAL("Not implemented yet");
		return VECTOR3_ZERO;
	}

	bool controllerDoesCollideUp(ControllerInstance /*i*/) const
	{
		RIO_FATAL("Not implemented yet");
		return false;
	}

	bool controllerDoesCollideDown(ControllerInstance /*i*/) const
	{
		RIO_FATAL("Not implemented yet");
		return false;
	}

	bool controllerDoesCollideSides(ControllerInstance /*i*/) const
	{
		RIO_FATAL("Not implemented yet");
		return false;
	}

	JointInstance jointCreate(ActorInstance a0, ActorInstance a1, const JointDesc& jointDesc)
	{
		const btVector3 anchor0 = getBtVector3(jointDesc.anchor0);
		const btVector3 anchor1 = getBtVector3(jointDesc.anchor1);
		btRigidBody* actor0 = actorList[a0.i].actor;
		btRigidBody* actor1 = getIsValid(a1) ? actorList[a1.i].actor : nullptr;

		btTypedConstraint* joint = nullptr;
		switch (jointDesc.type)
		{
			case JointType::FIXED:
			{
				const btTransform frame0 = btTransform(btQuaternion::getIdentity(), anchor0);
				const btTransform frame1 = btTransform(btQuaternion::getIdentity(), anchor1);
	 			joint = RIO_NEW(*allocator, btFixedConstraint)(*actor0
	 				, *actor1
	 				, frame0
	 				, frame1
	 				);
			}
			break;
			case JointType::SPRING:
			{
				joint = RIO_NEW(*allocator, btPoint2PointConstraint)(*actor0
					, *actor1
					, anchor0
					, anchor1
					);
			}
			break;
			case JointType::HINGE:
			{
				btHingeConstraint* hinge = RIO_NEW(*allocator, btHingeConstraint)(*actor0
					, *actor1
					, anchor0
					, anchor1
					, getBtVector3(jointDesc.hinge.axis)
					, getBtVector3(jointDesc.hinge.axis)
					);

				hinge->enableAngularMotor(jointDesc.hinge.useMotor
					, jointDesc.hinge.targetVelocity
					, jointDesc.hinge.maxMotorImpulse
					);

				hinge->setLimit(jointDesc.hinge.lowerLimit
					, jointDesc.hinge.upperLimit
					, jointDesc.hinge.bounciness
					);

				joint = hinge;
			}
			break;
			default:
			{
				RIO_FATAL("Unknown joint type");
			}
			break;
		}

		joint->setBreakingImpulseThreshold(jointDesc.breakForce);
		discreteDynamicsWorld->addConstraint(joint);

		return makeJointInstance(UINT32_MAX);
	}

	void jointDestroy(JointInstance /*i*/)
	{
		RIO_FATAL("Not implemented yet");
	}

	void raycast(const Vector3& from, const Vector3& direction, float length, RaycastMode::Enum mode, Array<RaycastHit>& hits)
	{
		const btVector3 start = getBtVector3(from);
		const btVector3 end = getBtVector3(from + direction*length);

		switch (mode)
		{
			case RaycastMode::CLOSEST:
			{
				btCollisionWorld::ClosestRayResultCallback closestRayResultCallback(start, end);
				discreteDynamicsWorld->rayTest(start, end, closestRayResultCallback);
				ArrayFn::resize(hits, 1);

				if (closestRayResultCallback.hasHit() == true)
				{
					hits[0].position = getVector3(closestRayResultCallback.m_hitPointWorld);
					hits[0].normal = getVector3(closestRayResultCallback.m_hitNormalWorld);
					hits[0].actor.i = (uint32_t)(uintptr_t)btRigidBody::upcast(closestRayResultCallback.m_collisionObject)->getUserPointer();
				}
			}
			break;
			case RaycastMode::ALL:
			{
				btCollisionWorld::AllHitsRayResultCallback closestRayResultCallback(start, end);
				discreteDynamicsWorld->rayTest(start, end, closestRayResultCallback);

				if (closestRayResultCallback.hasHit() == true)
				{
					const int hitPointCount = closestRayResultCallback.m_hitPointWorld.size();
					ArrayFn::resize(hits, hitPointCount);

					for (int i = 0; i < hitPointCount; ++i)
					{
						hits[i].position = getVector3(closestRayResultCallback.m_hitPointWorld[i]);
						hits[i].normal = getVector3(closestRayResultCallback.m_hitNormalWorld[i]);
						hits[i].actor.i = (uint32_t)(uintptr_t)btRigidBody::upcast(closestRayResultCallback.m_collisionObjects[i])->getUserPointer();
					}
				}
			}
			break;
			default:
			{
				RIO_FATAL("Unknown raycast mode");
			}
			break;
		}
	}

	Vector3 getGravity() const
	{
		return getVector3(discreteDynamicsWorld->getGravity());
	}

	void setGravity(const Vector3& gravity)
	{
		discreteDynamicsWorld->setGravity(getBtVector3(gravity));
	}

	void updateActorWorldPoses(const UnitId* begin, const UnitId* end, const Matrix4x4* beginWorld)
	{
		for (; begin != end; ++begin, ++beginWorld)
		{
			const uint32_t actorIndex = HashMapFn::get(actorMap, *begin, UINT32_MAX);
			if (actorIndex == UINT32_MAX)
			{
				continue;
			}

			const Quaternion rotation = getRotationAsQuaternion(*beginWorld);
			const Vector3 position = getTranslation(*beginWorld);
			actorList[actorIndex].actor->getMotionState()->setWorldTransform(btTransform(getBtQuaternion(rotation), getBtVector3(position)));
		}
	}

	void update(float dt)
	{
		discreteDynamicsWorld->stepSimulation(dt);

		const int collisionObjectsCount = discreteDynamicsWorld->getNumCollisionObjects();
		const btCollisionObjectArray& collisionArray = discreteDynamicsWorld->getCollisionObjectArray();
	    // Update actors
		for (int i = 0; i < collisionObjectsCount; ++i)
		{
			if ((uintptr_t)collisionArray[i]->getUserPointer() == (uintptr_t)UINT32_MAX)
			{
				continue;
			}

			btRigidBody* body = btRigidBody::upcast(collisionArray[i]);
			if (body
				&& body->getMotionState()
				&& body->isActive()
				)
			{
				const UnitId unitId = actorList[(uint32_t)(uintptr_t)body->getUserPointer()].unitId;

				btTransform transform;
				body->getMotionState()->getWorldTransform(transform);

				postTransformEvent(unitId
					, getVector3(transform.getOrigin())
					, getQuaternion(transform.getRotation())
					);
			}
		}
	}

	EventStream& getEventStream()
	{
		return eventStream;
	}

	virtual void debugDraw() override
	{
		if (!isDebugDrawing)
		{
			return;
		}

		discreteDynamicsWorld->debugDrawWorld();
		debugDrawer.debugLine->submit();
		debugDrawer.debugLine->reset();
	}

	void enableDebugDrawing(bool enable)
	{
		isDebugDrawing = enable;
	}

	void tickCallback(btDynamicsWorld* world, btScalar /*dt*/)
	{
		// Limit bodies velocity
		for (uint32_t i = 0; i < ArrayFn::getCount(actorList); ++i)
		{
			RIO_ASSERT_NOT_NULL(actorList[i].actor);
			const btVector3 velocity = actorList[i].actor->getLinearVelocity();
			const btScalar speed = velocity.length();

			if (speed > 100.0f)
			{
				actorList[i].actor->setLinearVelocity(velocity * 100.0f / speed);
			}
		}

		// Check collisions
		int manifoldsCount = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < manifoldsCount; ++i)
		{
			const btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);

			const btCollisionObject* actorA = contactManifold->getBody0();
			const btCollisionObject* actorB = contactManifold->getBody1();
			const ActorInstance a0 = makeActorInstance((uint32_t)(uintptr_t)actorA->getUserPointer());
			const ActorInstance a1 = makeActorInstance((uint32_t)(uintptr_t)actorB->getUserPointer());

			int contactsCount = contactManifold->getNumContacts();
			for (int j = 0; j < contactsCount; ++j)
			{
				const btManifoldPoint& point = contactManifold->getContactPoint(j);
				if (point.getDistance() < 0.0f)
				{
					const btVector3& whereA = point.getPositionWorldOnA();
					const btVector3& whereB = point.getPositionWorldOnB();
					const btVector3& normal = point.m_normalWorldOnB;

					postCollisionEvent(a0
						, a1
						, getVector3(whereA)
						, getVector3(normal)
						, point.getLifeTime() > 0
							? PhysicsCollisionEvent::BEGIN_TOUCH
							: PhysicsCollisionEvent::END_TOUCH   // TODO improve
						);
				}
			}
		}
	}

	void unitDestroyedCallback(UnitId unitId)
	{
		{
			ActorInstance firstActor = actorGet(unitId);
			if (getIsValid(firstActor))
			{
				actorDestroy(firstActor);
			}
		}

		{
			ColliderInstance currentCollider = colliderGetFirst(unitId);
			ColliderInstance nextCollider;

			while (getIsValid(currentCollider))
			{
				nextCollider = colliderGetNext(currentCollider);
				colliderDestroy(currentCollider);
				currentCollider = nextCollider;
			}
		}
	}

	static void tickCallbackWrapper(btDynamicsWorld* world, btScalar dt)
	{
		BulletWorld* bulletWorld = static_cast<BulletWorld*>(world->getWorldUserInfo());
		bulletWorld->tickCallback(world, dt);
	}

	static void unitDestroyedCallback(UnitId unitId, void* userPtr)
	{
		((BulletWorld*)userPtr)->unitDestroyedCallback(unitId);
	}

private:

	bool getIsValid(ColliderInstance i) 
	{ 
		return i.i != UINT32_MAX; 
	}
	bool getIsValid(ActorInstance i) 
	{ 
		return i.i != UINT32_MAX; 
	}
	bool getIsValid(ControllerInstance i) 
	{ 
		return i.i != UINT32_MAX; 
	}
	bool getIsValid(JointInstance i) 
	{ 
		return i.i != UINT32_MAX; 
	}

	ColliderInstance makeColliderInstance(uint32_t i) { ColliderInstance colliderInstance = { i }; return colliderInstance; }
	ActorInstance makeActorInstance(uint32_t i) { ActorInstance actorInstance = { i }; return actorInstance; }
	ControllerInstance makeControllerInstance(uint32_t i) { ControllerInstance controllerInstance = { i }; return controllerInstance; }
	JointInstance makeJointInstance(uint32_t i) { JointInstance jointInstance = { i }; return jointInstance; }

	void postCollisionEvent(ActorInstance a0, ActorInstance a1, const Vector3& where, const Vector3& normal, PhysicsCollisionEvent::Type type)
	{
		PhysicsCollisionEvent ev;
		ev.type = type;
		ev.actors[0] = a0;
		ev.actors[1] = a1;
		ev.where = where;
		ev.normal = normal;

		EventStreamFn::write(eventStream, EventType::PHYSICS_COLLISION, ev);
	}

	void postTriggerEvent(ActorInstance trigger, ActorInstance other, PhysicsTriggerEvent::Type type)
	{
		PhysicsTriggerEvent ev;
		ev.type = type;
		ev.trigger = trigger;
		ev.other = other;

		EventStreamFn::write(eventStream, EventType::PHYSICS_TRIGGER, ev);
	}

	void postTransformEvent(UnitId unitId, const Vector3& position, const Quaternion& rotation)
	{
		PhysicsTransformEvent ev;
		ev.unitId = unitId;
		ev.position = position;
		ev.rotation = rotation;

		EventStreamFn::write(eventStream, EventType::PHYSICS_TRANSFORM, ev);
	}

	struct ColliderInstanceData
	{
		UnitId unitId;
		Matrix4x4 localTransformMatrix;
		btTriangleIndexVertexArray* vertexArray;
		btCollisionShape* shape;
		ColliderInstance next;
	};

	struct ActorInstanceData
	{
		UnitId unitId;
		btRigidBody* actor;
	};

	struct ControllerInstanceData
	{
		UnitId unitId;
		btKinematicCharacterController* kinematicCharacterController;
	};

	Allocator* allocator;
	UnitManager* unitManager;

	HashMap<UnitId, uint32_t> colliderMap;
	HashMap<UnitId, uint32_t> actorMap;
	HashMap<UnitId, uint32_t> controllerMap;
	Array<ColliderInstanceData> colliderList;
	Array<ActorInstanceData> actorList;
	Array<ControllerInstanceData> controllerList;
	Array<btTypedConstraint*> jointList;

	OverlapFilterCallback overlapFilterCallback;
	btDiscreteDynamicsWorld* discreteDynamicsWorld = nullptr;
	DebugDrawer debugDrawer;

	EventStream eventStream;

	const PhysicsConfigResource* physicsConfigResource;
	bool isDebugDrawing = false;
};

namespace PhysicsWorldFn
{
	PhysicsWorld* create(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager, DebugLine& debugLine)
	{
		return RIO_NEW(a, BulletWorld)(a, resourceManager, unitManager, debugLine);
	}

	void destroy(Allocator& a, PhysicsWorld* physicsWorld)
	{
		RIO_DELETE(a, physicsWorld);
	}
} // namespace PhysicsWorldFn

} // namespace Rio

#endif // RIO_PHYSICS_BULLET
