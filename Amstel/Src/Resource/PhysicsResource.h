// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Containers/ContainerTypes.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

namespace PhysicsResourceInternalFn
{
	inline void compile(const char* /*path*/, CompileOptions& /*compileOptions*/) {}
	inline void* load(File& /*file*/, Allocator& /*a*/) { return nullptr; }
	inline void unload(Allocator& /*a*/, void* /*resource*/) {}
	Buffer compileController(const char* json, CompileOptions& compileOptions);
	Buffer compileCollider(const char* json, CompileOptions& compileOptions);
	Buffer compileActor(const char* json, CompileOptions& compileOptions);
	Buffer compileJoint(const char* json, CompileOptions& compileOptions);
} // namespace PhysicsResourceInternalFn

struct PhysicsConfigResource
{
	uint32_t version;
	uint32_t materialListCount;
	uint32_t materialsOffset;
	uint32_t shapeListCount;
	uint32_t shapesOffset;
	uint32_t actorListCount;
	uint32_t actorsOffset;
	uint32_t filterListCount;
	uint32_t filtersOffset;
};

struct PhysicsConfigMaterial
{
	StringId32 name;
	float staticFriction;
	float dynamicFriction;
	float restitution;
};

struct PhysicsCollisionFilter
{
	StringId32 name;
	uint32_t me;
	uint32_t mask;
};

struct PhysicsConfigShape
{
	StringId32 name;
	bool trigger;
	char _pad[3];
};

struct PhysicsConfigActor
{
	enum
	{
		DYNAMIC         = 1 << 0,
		KINEMATIC       = 1 << 1,
		DISABLE_GRAVITY = 1 << 2
	};

	StringId32 name;
	float linearDamping;
	float angularDamping;
	uint32_t flags;
};

namespace PhysicsConfigResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace PhysicsConfigResourceInternalFn

namespace PhysicsConfigResourceFn
{
	const PhysicsConfigMaterial* getPhysicsConfigMaterial(const PhysicsConfigResource* physicsConfigResource, StringId32 name);
	const PhysicsConfigShape* getPhysicsConfigShape(const PhysicsConfigResource* physicsConfigResource, StringId32 name);
	const PhysicsConfigActor* getPhysicsConfigActor(const PhysicsConfigResource* physicsConfigResource, StringId32 name);
	const PhysicsCollisionFilter* getPhysicsCollisionFilter(const PhysicsConfigResource* physicsConfigResource, StringId32 name);
} // namespace PhysicsConfigResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka