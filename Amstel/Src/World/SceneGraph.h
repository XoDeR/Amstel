// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Base/Types.h"
#include "World/WorldTypes.h"

namespace Rio
{

// Collection of nodes, possibly linked together to form a tree
struct SceneGraph
{
private:
	uint32_t marker = 0;
public:

	struct Pose
	{
		Vector3 position;
		Matrix3x3 rotation;
		Vector3 scale;

		Pose& operator=(const Matrix4x4& m);
	};

	struct InstanceData
	{
		uint32_t size = 0;
		uint32_t capacity = 0;
		void* buffer = nullptr;

		UnitId* unit = nullptr;
		Matrix4x4* world = nullptr;
		Pose* local = nullptr;
		TransformInstance* parent = nullptr;
		TransformInstance* firstChild = nullptr;
		TransformInstance* nextSibling = nullptr;
		TransformInstance* prevSibling = nullptr;
		bool* changed = nullptr;
	};

	SceneGraph(Allocator& a);
	~SceneGraph();

	void grow();
	void allocate(uint32_t instancesCount);
	TransformInstance makeInstance(uint32_t i);
	// Creates a new transform instance for unit <id>
	TransformInstance create(UnitId id, const Matrix4x4& pose);
	// Creates a new transform instance for unit <id>
	TransformInstance create(UnitId id, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
	// Destroys the transform <i>
	void destroy(TransformInstance i);
	// Returns the transform instance of unit <id>
	TransformInstance get(UnitId id);
	// Sets the local position, rotation, scale or pose of the given node
	void setLocalPosition(TransformInstance i, const Vector3& position);
	void setLocalRotation(TransformInstance i, const Quaternion& rotation);
	void setLocalScale(TransformInstance i, const Vector3& scale);
	void setLocalPose(TransformInstance i, const Matrix4x4& pose);

	// Returns the local position, rotation or pose of the given node
	Vector3 getLocalPosition(TransformInstance i) const;
	Quaternion getLocalRotation(TransformInstance i) const;
	Vector3 getLocalScale(TransformInstance i) const;
	Matrix4x4 getLocalPose(TransformInstance i) const;

	// Returns the world position, rotation or pose of the given node
	Vector3 getWorldPosition(TransformInstance i) const;
	Quaternion getWorldRotation(TransformInstance i) const;
	Matrix4x4 getWorldPose(TransformInstance i) const;
	void setWorldPose(TransformInstance i, const Matrix4x4& pose);

	uint32_t getNodeCount() const;

	// Links the <child> node to the <parent> node
	void link(TransformInstance child, TransformInstance parent);

	// Unlinks the <child> node from its parent if it has any
	// After unlinking, the <child> local pose is set to its previous world pose
	void unlink(TransformInstance child);
	void clearChanged();
	void getChanged(Array<UnitId>& units, Array<Matrix4x4>& worldPoseList);
	bool getIsValid(TransformInstance i);
	void setLocal(TransformInstance i);
	void transform(const Matrix4x4& parent, TransformInstance i);

	Allocator& allocator;
	InstanceData instanceData;
	HashMap<UnitId, uint32_t> unitIdMap;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka