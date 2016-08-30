// Copyright (c) 2016 Volodymyr Syvochka
#include "World/SceneGraph.h"
#include "Core/Memory/Allocator.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

#include <stdint.h> // UINT_MAX
#include <string.h> // memcpy

namespace Rio
{

SceneGraph::Pose& SceneGraph::Pose::operator=(const Matrix4x4& m)
{
	Matrix3x3 rotationMatrix = getMatrix3x3(m);
	normalize(rotationMatrix.x);
	normalize(rotationMatrix.y);
	normalize(rotationMatrix.z);

	position = getTranslation(m);
	rotation = rotationMatrix;
	scale = Rio::getScale(m);
	return *this;
}

SceneGraph::SceneGraph(Allocator& a)
	: marker(SCENE_GRAPH_MARKER)
	, allocator(a)
	, unitIdMap(a)
{
}

SceneGraph::~SceneGraph()
{
	allocator.deallocate(instanceData.buffer);
	marker = 0;
}

TransformInstance SceneGraph::makeInstance(uint32_t i)
{
	TransformInstance transformInstance = { i };
	return transformInstance;
}

void SceneGraph::allocate(uint32_t instancesCount)
{
	RIO_ASSERT(instancesCount > this->instanceData.size, "instancesCount > instanceData.size");

	const uint32_t bytes = 0
		+ instancesCount * sizeof(UnitId) + alignof(UnitId)
		+ instancesCount * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ instancesCount * sizeof(Pose) + alignof(Pose)
		+ instancesCount * sizeof(TransformInstance) * 4 + alignof(TransformInstance)
		+ instancesCount * sizeof(bool) + alignof(bool)
		;

	InstanceData newInstanceData;
	newInstanceData.size = this->instanceData.size;
	newInstanceData.capacity = instancesCount;
	newInstanceData.buffer = allocator.allocate(bytes);

	newInstanceData.unit = (UnitId*)(newInstanceData.buffer);
	newInstanceData.world = (Matrix4x4*)MemoryFn::alignTop(newInstanceData.unit + instancesCount, alignof(Matrix4x4));
	newInstanceData.local = (Pose*)MemoryFn::alignTop(newInstanceData.world + instancesCount, alignof(Pose));
	newInstanceData.parent = (TransformInstance*)MemoryFn::alignTop(newInstanceData.local + instancesCount, alignof(TransformInstance));
	newInstanceData.firstChild = (TransformInstance*)MemoryFn::alignTop(newInstanceData.parent + instancesCount, alignof(TransformInstance));
	newInstanceData.nextSibling = (TransformInstance*)MemoryFn::alignTop(newInstanceData.firstChild + instancesCount, alignof(TransformInstance));
	newInstanceData.prevSibling = (TransformInstance*)MemoryFn::alignTop(newInstanceData.nextSibling + instancesCount, alignof(TransformInstance));
	newInstanceData.changed = (bool*)MemoryFn::alignTop(newInstanceData.prevSibling + instancesCount, alignof(bool));

	memcpy(newInstanceData.unit, this->instanceData.unit, this->instanceData.size * sizeof(UnitId));
	memcpy(newInstanceData.world, this->instanceData.world, this->instanceData.size * sizeof(Matrix4x4));
	memcpy(newInstanceData.local, this->instanceData.local, this->instanceData.size * sizeof(Pose));
	memcpy(newInstanceData.parent, this->instanceData.parent, this->instanceData.size * sizeof(TransformInstance));
	memcpy(newInstanceData.firstChild, this->instanceData.firstChild, this->instanceData.size * sizeof(TransformInstance));
	memcpy(newInstanceData.nextSibling, this->instanceData.nextSibling, this->instanceData.size * sizeof(TransformInstance));
	memcpy(newInstanceData.prevSibling, this->instanceData.prevSibling, this->instanceData.size * sizeof(TransformInstance));
	memcpy(newInstanceData.changed, this->instanceData.changed, this->instanceData.size * sizeof(bool));

	allocator.deallocate(this->instanceData.buffer);
	this->instanceData = newInstanceData;
}

TransformInstance SceneGraph::create(UnitId id, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	Matrix4x4 pose;
	setToIdentity(pose);
	setTranslation(pose, position);
	setRotation(pose, rotation);
	setScale(pose, scale);

	return create(id, pose);
}

TransformInstance SceneGraph::create(UnitId id, const Matrix4x4& pose)
{
	RIO_ASSERT(!HashMapFn::has(unitIdMap, id), "Unit already has transform");

	if (this->instanceData.capacity == this->instanceData.size)
	{
		grow();
	}

	const uint32_t last = this->instanceData.size;

	this->instanceData.unit[last] = id;
	this->instanceData.world[last] = pose;
	this->instanceData.local[last] = pose;
	this->instanceData.parent[last].i = UINT32_MAX;
	this->instanceData.firstChild[last].i = UINT32_MAX;
	this->instanceData.nextSibling[last].i = UINT32_MAX;
	this->instanceData.prevSibling[last].i = UINT32_MAX;
	this->instanceData.changed[last] = false;

	++this->instanceData.size;

	HashMapFn::set(unitIdMap, id, last);

	return makeInstance(last);
}

void SceneGraph::destroy(TransformInstance i)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");

	const uint32_t last = this->instanceData.size - 1;
	const UnitId unitId = this->instanceData.unit[i.i];
	const UnitId lastUnitId = this->instanceData.unit[last];

	this->instanceData.unit[i.i] = this->instanceData.unit[last];
	this->instanceData.world[i.i] = this->instanceData.world[last];
	this->instanceData.local[i.i] = this->instanceData.local[last];
	this->instanceData.parent[i.i] = this->instanceData.parent[last];
	this->instanceData.firstChild[i.i] = this->instanceData.firstChild[last];
	this->instanceData.nextSibling[i.i] = this->instanceData.nextSibling[last];
	this->instanceData.prevSibling[i.i] = this->instanceData.prevSibling[last];
	this->instanceData.changed[i.i] = this->instanceData.changed[last];

	HashMapFn::set(unitIdMap, lastUnitId, i.i);
	HashMapFn::remove(unitIdMap, unitId);

	--this->instanceData.size;
}

TransformInstance SceneGraph::get(UnitId id)
{
	return makeInstance(HashMapFn::get(unitIdMap, id, UINT32_MAX));
}

void SceneGraph::setLocalPosition(TransformInstance i, const Vector3& position)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	this->instanceData.local[i.i].position = position;
	setLocal(i);
}

void SceneGraph::setLocalRotation(TransformInstance i, const Quaternion& rotation)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	this->instanceData.local[i.i].rotation = createMatrix3x3(rotation);
	setLocal(i);
}

void SceneGraph::setLocalScale(TransformInstance i, const Vector3& scale)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	this->instanceData.local[i.i].scale = scale;
	setLocal(i);
}

void SceneGraph::setLocalPose(TransformInstance i, const Matrix4x4& pose)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	this->instanceData.local[i.i] = pose;
	setLocal(i);
}

Vector3 SceneGraph::getLocalPosition(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return this->instanceData.local[i.i].position;
}

Quaternion SceneGraph::getLocalRotation(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return createQuaternion(this->instanceData.local[i.i].rotation);
}

Vector3 SceneGraph::getLocalScale(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return this->instanceData.local[i.i].scale;
}

Matrix4x4 SceneGraph::getLocalPose(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	Matrix4x4 transform = createMatrix4x4(createQuaternion(this->instanceData.local[i.i].rotation), this->instanceData.local[i.i].position);
	setScale(transform, this->instanceData.local[i.i].scale);
	return transform;
}

Vector3 SceneGraph::getWorldPosition(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return getTranslation(this->instanceData.world[i.i]);
}

Quaternion SceneGraph::getWorldRotation(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return getRotationAsQuaternion(this->instanceData.world[i.i]);
}

Matrix4x4 SceneGraph::getWorldPose(TransformInstance i) const
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	return this->instanceData.world[i.i];
}

void SceneGraph::setWorldPose(TransformInstance i, const Matrix4x4& pose)
{
	RIO_ASSERT(i.i < this->instanceData.size, "Index out of bounds");
	this->instanceData.world[i.i] = pose;
	this->instanceData.changed[i.i] = true;
}

uint32_t SceneGraph::getNodeCount() const
{
	return this->instanceData.size;
}

void SceneGraph::link(TransformInstance child, TransformInstance parent)
{
	RIO_ASSERT(child.i < this->instanceData.size, "Index out of bounds");
	RIO_ASSERT(parent.i < this->instanceData.size, "Index out of bounds");

	unlink(child);

	if (getIsValid(this->instanceData.firstChild[parent.i]) == false)
	{
		this->instanceData.firstChild[parent.i] = child;
		this->instanceData.parent[child.i] = parent;
	}
	else
	{
		TransformInstance prev = { UINT32_MAX };
		TransformInstance node = this->instanceData.firstChild[parent.i];
		while (getIsValid(node) == true)
		{
			prev = node;
			node = this->instanceData.nextSibling[node.i];
		}

		this->instanceData.nextSibling[prev.i] = child;

		this->instanceData.firstChild[child.i].i = UINT32_MAX;
		this->instanceData.nextSibling[child.i].i = UINT32_MAX;
		this->instanceData.prevSibling[child.i] = prev;
	}

	Matrix4x4 parentTransform = this->instanceData.world[parent.i];
	Matrix4x4 childTransform = this->instanceData.world[child.i];
	const Vector3 childScale = getScale(childTransform);

	Vector3 xParent = getAxisX(parentTransform);
	Vector3 yParent = getAxisY(parentTransform);
	Vector3 zParent = getAxisZ(parentTransform);
	Vector3 xChild = getAxisX(childTransform);
	Vector3 yChild = getAxisY(childTransform);
	Vector3 zChild = getAxisZ(childTransform);

	setAxisX(parentTransform, normalize(xParent));
	setAxisY(parentTransform, normalize(yParent));
	setAxisZ(parentTransform, normalize(zParent));
	setAxisX(childTransform, normalize(xChild));
	setAxisY(childTransform, normalize(yChild));
	setAxisZ(childTransform, normalize(zChild));

	const Matrix4x4 relativeTransform = childTransform * getInverted(parentTransform);

	this->instanceData.local[child.i].position = getTranslation(relativeTransform);
	this->instanceData.local[child.i].rotation = getMatrix3x3(relativeTransform);
	this->instanceData.local[child.i].scale = childScale;
	this->instanceData.parent[child.i] = parent;

	transform(parentTransform, child);
}

void SceneGraph::unlink(TransformInstance child)
{
	RIO_ASSERT(child.i < this->instanceData.size, "Index out of bounds");

	if (!getIsValid(this->instanceData.parent[child.i]))
	{
		return;
	}
	if (!getIsValid(this->instanceData.prevSibling[child.i]))
	{
		this->instanceData.firstChild[this->instanceData.parent[child.i].i] = this->instanceData.nextSibling[child.i];
	}
	else
	{
		this->instanceData.nextSibling[this->instanceData.prevSibling[child.i].i] = this->instanceData.nextSibling[child.i];
	}

	if (getIsValid(this->instanceData.nextSibling[child.i]))
	{
		this->instanceData.prevSibling[this->instanceData.nextSibling[child.i].i] = this->instanceData.prevSibling[child.i];
	}

	this->instanceData.parent[child.i].i = UINT32_MAX;
	this->instanceData.nextSibling[child.i].i = UINT32_MAX;
	this->instanceData.prevSibling[child.i].i = UINT32_MAX;
}

void SceneGraph::clearChanged()
{
	for (uint32_t i = 0; i < this->instanceData.size; ++i)
	{
		this->instanceData.changed[i] = false;
	}
}

void SceneGraph::getChanged(Array<UnitId>& units, Array<Matrix4x4>& worldPoseList)
{
	for (uint32_t i = 0; i < this->instanceData.size; ++i)
	{
		if (this->instanceData.changed[i])
		{
			ArrayFn::pushBack(units, this->instanceData.unit[i]);
			ArrayFn::pushBack(worldPoseList, this->instanceData.world[i]);
		}
	}
}

bool SceneGraph::getIsValid(TransformInstance i)
{
	return i.i != UINT32_MAX;
}

void SceneGraph::setLocal(TransformInstance i)
{
	TransformInstance parent = this->instanceData.parent[i.i];
	Matrix4x4 parentTransformMatrix = getIsValid(parent) ? this->instanceData.world[parent.i] : MATRIX4X4_IDENTITY;
	transform(parentTransformMatrix, i);

	this->instanceData.changed[i.i] = true;
}

void SceneGraph::transform(const Matrix4x4& parent, TransformInstance i)
{
	this->instanceData.world[i.i] = getLocalPose(i) * parent;

	TransformInstance child = this->instanceData.firstChild[i.i];
	while (getIsValid(child))
	{
		transform(this->instanceData.world[i.i], child);
		child = this->instanceData.nextSibling[child.i];
	}
}

void SceneGraph::grow()
{
	allocate(this->instanceData.capacity * 2 + 1);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka