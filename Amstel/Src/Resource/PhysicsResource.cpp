// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/PhysicsResource.h"

#include "Core/Math/Aabb.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringUtils.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Base/Macros.h"
#include "Core/Containers/Map.h"
#include "Core/Math/Quaternion.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Math/Sphere.h"

#include "Resource/CompileOptions.h"

#include "World/WorldTypes.h"

namespace Rio
{

namespace PhysicsResourceInternalFn
{
	struct ColliderInfo
	{
		const char* name;
		ColliderType::Enum type;
	};

	static const ColliderInfo colliderInfoMap[] =
	{
		{ "sphere", ColliderType::SPHERE },
		{ "capsule", ColliderType::CAPSULE },
		{ "box", ColliderType::BOX },
		{ "convexHull", ColliderType::CONVEX_HULL },
		{ "mesh", ColliderType::MESH },
		{ "heightfield", ColliderType::HEIGHTFIELD }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(colliderInfoMap) == ColliderType::COUNT);

	struct JointInfo
	{
		const char* name;
		JointType::Enum type;
	};

	static const JointInfo jointInfoMap[] =
	{
		{ "fixed",  JointType::FIXED  },
		{ "hinge",  JointType::HINGE  },
		{ "spring", JointType::SPRING }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(jointInfoMap) == JointType::COUNT);

	static ColliderType::Enum getColliderTypeFromShapeType(const char* type)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(colliderInfoMap); ++i)
		{
			if (strcmp(type, colliderInfoMap[i].name) == 0)
			{
				return colliderInfoMap[i].type;
			}
		}

		return ColliderType::COUNT;
	}

	static JointType::Enum getJointTypeFromName(const char* type)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(jointInfoMap); ++i)
		{
			if (strcmp(type, jointInfoMap[i].name) == 0)
			{
				return jointInfoMap[i].type;
			}
		}

		return JointType::COUNT;
	}

	Buffer compileController(const char* json, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		ControllerDesc controllerDesc;
		controllerDesc.height = JsonRFn::parseFloat(jsonObject["height"]);
		controllerDesc.radius = JsonRFn::parseFloat(jsonObject["radius"]);
		controllerDesc.slopeLimit = JsonRFn::parseFloat(jsonObject["slopeLimit"]);
		controllerDesc.stepOffset = JsonRFn::parseFloat(jsonObject["stepOffset"]);
		controllerDesc.contactOffset = JsonRFn::parseFloat(jsonObject["contactOffset"]);
		controllerDesc.collisionFilter = JsonRFn::parseStringId(jsonObject["collisionFilter"]);

		Buffer buffer(getDefaultAllocator());
		ArrayFn::push(buffer, (char*)&controllerDesc, sizeof(controllerDesc));
		return buffer;
	}

	void compileSphere(const Array<Vector3>& points, ColliderDesc& colliderDesc)
	{
		Sphere sphere;
		SphereFn::reset(sphere);
		SphereFn::addPoints(sphere, ArrayFn::getCount(points), ArrayFn::begin(points));

		colliderDesc.sphere.radius = sphere.r;
	}

	void compileCapsule(const Array<Vector3>& points, ColliderDesc& colliderDesc)
	{
		Aabb aabb;
		AabbFn::reset(aabb);
		AabbFn::addPoints(aabb, ArrayFn::getCount(points), ArrayFn::begin(points));

		colliderDesc.capsule.radius = AabbFn::getRadius(aabb) / 2.0f;
		colliderDesc.capsule.height = (aabb.max.y - aabb.min.y) / 2.0f;
	}

	void compileBox(const Array<Vector3>& points, ColliderDesc& colliderDesc)
	{
		Aabb aabb;
		AabbFn::reset(aabb);
		AabbFn::addPoints(aabb, ArrayFn::getCount(points), ArrayFn::begin(points));

		colliderDesc.box.halfSize = (aabb.max - aabb.min) * 0.5f;
	}

	Buffer compileCollider(const char* json, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		DynamicString type(ta);
		JsonRFn::parseString(jsonObject["shape"], type);

		ColliderType::Enum colliderType = getColliderTypeFromShapeType(type.getCStr());
		RESOURCE_COMPILER_ASSERT(colliderType != ColliderType::COUNT
			, compileOptions
			, "Unknown shape type: '%s'"
			, type.getCStr()
			);

		ColliderDesc colliderDesc;
		colliderDesc.type = colliderType;
		colliderDesc.shapeClass = JsonRFn::parseStringId(jsonObject["class"]);
		colliderDesc.material = JsonRFn::parseStringId(jsonObject["material"]);
		colliderDesc.localTransformMatrix = MATRIX4X4_IDENTITY;
		colliderDesc.size = 0;

		// Parse .mesh
		DynamicString scene(ta);
		DynamicString name(ta);
		JsonRFn::parseString(jsonObject["scene"], scene);
		JsonRFn::parseString(jsonObject["name"], name);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_MESH, scene.getCStr(), compileOptions);
		scene += "." RESOURCE_EXTENSION_MESH;

		Buffer file = compileOptions.read(scene.getCStr());
		JsonObject meshJsonObject(ta);
		JsonObject geometries(ta);
		JsonObject geometry(ta);
		JsonObject nodes(ta);
		JsonObject node(ta);
		JsonRFn::parse(file, meshJsonObject);
		JsonRFn::parse(meshJsonObject["geometries"], geometries);
		// TODO
		RESOURCE_COMPILER_ASSERT(json_object::has(geometries, name.c_str())
			, opts
			, "Geometry '%s' does not exist"
			, name.c_str()
			);
		JsonRFn::parse(geometries[name.getCStr()], geometry);
		JsonRFn::parse(meshJsonObject["nodes"], nodes);
		// TODO
		RESOURCE_COMPILER_ASSERT(json_object::has(nodes, name.c_str())
			, opts
			, "Node '%s' does not exist"
			, name.c_str()
			);
		JsonRFn::parse(nodes[name.getCStr()], node);

		Matrix4x4 localMatrix = JsonRFn::parseMatrix4x4(node["localMatrix"]);
		colliderDesc.localTransformMatrix = localMatrix;

		JsonArray positionList(ta);
		JsonRFn::parseArray(geometry["position"], positionList);

		JsonObject indices(ta);
		JsonArray indicesData(ta);
		JsonArray positionIndexList(ta);
		JsonRFn::parseObject(geometry["indices"], indices);
		JsonRFn::parseArray(indices["data"], indicesData);
		JsonRFn::parseArray(indicesData[0], positionIndexList);

		Array<Vector3> points(getDefaultAllocator());
		for (uint32_t i = 0; i < ArrayFn::getCount(positionList); i += 3)
		{
			Vector3 p;
			p.x = JsonRFn::parseFloat(positionList[i + 0]);
			p.y = JsonRFn::parseFloat(positionList[i + 1]);
			p.z = JsonRFn::parseFloat(positionList[i + 2]);
			ArrayFn::pushBack(points, p*localMatrix);
		}

		Array<uint16_t> pointIndexList(getDefaultAllocator());
		for (uint32_t i = 0; i < ArrayFn::getCount(positionIndexList); ++i)
		{
			ArrayFn::pushBack(pointIndexList, (uint16_t)JsonRFn::parseInt(positionIndexList[i]));
		}

		switch (colliderDesc.type)
		{
			case ColliderType::SPHERE:      compileSphere(points, colliderDesc); break;
			case ColliderType::CAPSULE:     compileCapsule(points, colliderDesc); break;
			case ColliderType::BOX:         compileBox(points, colliderDesc); break;
			case ColliderType::CONVEX_HULL: break;
			case ColliderType::MESH:        break;
			case ColliderType::HEIGHTFIELD:
			{
				RESOURCE_COMPILER_ASSERT(false, compileOptions, "Not implemented yet");
				break;
			}
		}

		const uint32_t pointsCount  = ArrayFn::getCount(points);
		const uint32_t indicesCount = ArrayFn::getCount(pointIndexList);

		const bool needsPoints = colliderDesc.type == ColliderType::CONVEX_HULL
			|| colliderDesc.type == ColliderType::MESH;

		colliderDesc.size += (needsPoints ? sizeof(uint32_t) + sizeof(Vector3) * ArrayFn::getCount(points) : 0);
		colliderDesc.size += (colliderDesc.type == ColliderType::MESH ? sizeof(uint32_t) + sizeof(uint16_t) * ArrayFn::getCount(pointIndexList) : 0);

		Buffer buffer(getDefaultAllocator());
		ArrayFn::push(buffer, (char*)&colliderDesc, sizeof(colliderDesc));

		if (needsPoints == true)
		{
			ArrayFn::push(buffer, (char*)&pointsCount, sizeof(pointsCount));
			ArrayFn::push(buffer, (char*)ArrayFn::begin(points), sizeof(Vector3) * ArrayFn::getCount(points));
		}
		if (colliderDesc.type == ColliderType::MESH)
		{
			ArrayFn::push(buffer, (char*)&indicesCount, sizeof(indicesCount));
			ArrayFn::push(buffer, (char*)ArrayFn::begin(pointIndexList), sizeof(uint16_t) * ArrayFn::getCount(pointIndexList));
		}

		return buffer;
	}

	Buffer compileActor(const char* json, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		ActorResource actorResource;
		actorResource.actorClass = JsonRFn::parseStringId(jsonObject["class"]);
		actorResource.mass = JsonRFn::parseFloat(jsonObject["mass"]);
		actorResource.collisionFilter = JsonRFn::parseStringId(jsonObject["collisionFilter"]);

		actorResource.flags = 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockTranslationX") ? JsonRFn::parseBool(jsonObject["lockTranslationX"]) : 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockTranslationY") ? JsonRFn::parseBool(jsonObject["lockTranslationY"]) : 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockTranslationZ") ? JsonRFn::parseBool(jsonObject["lockTranslationZ"]) : 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockRotationX") ? JsonRFn::parseBool(jsonObject["lockRotationX"]) : 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockRotationY") ? JsonRFn::parseBool(jsonObject["lockRotationY"]) : 0;
		actorResource.flags |= JsonObjectFn::has(jsonObject, "lockRotationZ") ? JsonRFn::parseBool(jsonObject["lockRotationZ"]) : 0;

		Buffer buffer(getDefaultAllocator());
		ArrayFn::push(buffer, (char*)&actorResource, sizeof(actorResource));
		return buffer;
	}

	Buffer compileJoint(const char* json, CompileOptions& compileOptions)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		DynamicString type(ta);
		JsonRFn::parseString(jsonObject["type"], type);

		JointType::Enum jt = getJointTypeFromName(type.getCStr());
		RESOURCE_COMPILER_ASSERT(jt != JointType::COUNT
			, compileOptions
			, "Unknown joint type: '%s'"
			, type.getCStr()
			);

		JointDesc jointDesc;
		jointDesc.type     = jt;
		jointDesc.anchor0 = JsonRFn::parseVector3(jsonObject["anchor0"]);
		jointDesc.anchor1 = JsonRFn::parseVector3(jsonObject["anchor1"]);

		switch (jointDesc.type)
		{
			case JointType::HINGE:
			{
				jointDesc.hinge.useMotor = JsonRFn::parseBool (jsonObject["useMotor"]);
				jointDesc.hinge.targetVelocity = JsonRFn::parseFloat(jsonObject["targetVelocity"]);
				jointDesc.hinge.maxMotorImpulse = JsonRFn::parseFloat(jsonObject["maxMotorImpulse"]);
				jointDesc.hinge.lowerLimit = JsonRFn::parseFloat(jsonObject["lowerLimit"]);
				jointDesc.hinge.upperLimit = JsonRFn::parseFloat(jsonObject["upperLimit"]);
				jointDesc.hinge.bounciness = JsonRFn::parseFloat(jsonObject["bounciness"]);
				break;
			}
		}

		Buffer buffer(getDefaultAllocator());
		ArrayFn::push(buffer, (char*)&jointDesc, sizeof(jointDesc));
		return buffer;
	}
} // namespace PhysicsResourceInternalFn

namespace PhysicsConfigResourceInternalFn
{
	void parseMaterials(const char* json, Array<PhysicsConfigMaterial>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(json, object);

		auto begin = JsonObjectFn::begin(object);
		auto end = JsonObjectFn::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value = begin->pair.second;

			JsonObject material(ta);
			JsonRFn::parseObject(value, material);

			PhysicsConfigMaterial physicsConfigMaterial;
			physicsConfigMaterial.name = StringId32(key.getData(), key.getLength());
			physicsConfigMaterial.staticFriction = JsonRFn::parseFloat(material["staticFriction"]);
			physicsConfigMaterial.dynamicFriction = JsonRFn::parseFloat(material["dynamicFriction"]);
			physicsConfigMaterial.restitution = JsonRFn::parseFloat(material["restitution"]);

			ArrayFn::pushBack(objects, physicsConfigMaterial);
		}
	}

	void parseShapes(const char* json, Array<PhysicsConfigShape>& physicsConfigShapeList)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		auto begin = JsonObjectFn::begin(jsonObject);
		auto end = JsonObjectFn::end(jsonObject);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value = begin->pair.second;

			JsonObject shape(ta);
			JsonRFn::parseObject(value, shape);

			PhysicsConfigShape physicsConfigShape;
			physicsConfigShape.name = StringId32(key.getData(), key.getLength());
			physicsConfigShape.trigger = JsonRFn::parseBool(shape["trigger"]);

			ArrayFn::pushBack(physicsConfigShapeList, physicsConfigShape);
		}
	}

	void parseActors(const char* json, Array<PhysicsConfigActor>& objects)
	{
		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		auto begin = JsonObjectFn::begin(jsonObject);
		auto end = JsonObjectFn::end(jsonObject);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value = begin->pair.second;

			JsonObject actor(ta);
			JsonRFn::parseObject(value, actor);

			PhysicsConfigActor physicsConfigActor;
			physicsConfigActor.name = StringId32(key.getData(), key.getLength());
			// physicsConfigActor.linearDamping  = JsonRFn::parseFloat(actor["linearDamping"]);  // 0.0f;
			// physicsConfigActor.angularDamping = JsonRFn::parseFloat(actor["angularDamping"]); // 0.05f;

			const bool hasDynamic = JsonObjectFn::has(actor, "dynamic");
			const bool hasKinematic = JsonObjectFn::has(actor, "kinematic");
			const bool hasDisableGravity = JsonObjectFn::has(actor, "disableGravity");

			physicsConfigActor.flags = 0;

			if (hasDynamic == true)
			{
				physicsConfigActor.flags |= (JsonRFn::parseBool(actor["dynamic"])
					? 1
					: 0
					);
			}
			if (hasKinematic == true)
			{
				physicsConfigActor.flags |= (JsonRFn::parseBool(actor["kinematic"])
					? PhysicsConfigActor::KINEMATIC
					: 0
					);
			}
			if (hasDisableGravity == true)
			{
				physicsConfigActor.flags |= (JsonRFn::parseBool(actor["disableGravity"])
					? PhysicsConfigActor::DISABLE_GRAVITY
					: 0
					);
			}

			ArrayFn::pushBack(objects, physicsConfigActor);
		}
	}

	struct CollisionFilterCompiler
	{
		CollisionFilterCompiler(CompileOptions& compileOptions)
			: compileOptions(compileOptions)
			, filterMap(getDefaultAllocator())
			, filterList(getDefaultAllocator())
		{
		}

		void parse(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject jsonObject(ta);
			JsonRFn::parse(json, jsonObject);

			auto begin = JsonObjectFn::begin(jsonObject);
			auto end = JsonObjectFn::end(jsonObject);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const StringId32 id = StringId32(key.getData(), key.getLength());

				MapFn::set(filterMap, id, createNewFilterMask());
			}

			begin = JsonObjectFn::begin(jsonObject);
			end = JsonObjectFn::end(jsonObject);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const char* value = begin->pair.second;
				const StringId32 id = StringId32(key.getData(), key.getLength());

				TempAllocator4096 ta;
				JsonObject filter(ta);
				JsonRFn::parseObject(value, filter);

				JsonArray collidesWith(ta);
				JsonRFn::parseArray(filter["collidesWith"], collidesWith);

				uint32_t mask = 0;
				for (uint32_t i = 0; i < ArrayFn::getCount(collidesWith); ++i)
				{
					const StringId32 filterHash = JsonRFn::parseStringId(collidesWith[i]);
					mask |= getMaskFromFilter(filterHash);
				}

				// Build mask
				PhysicsCollisionFilter physicsCollisionFilter;
				physicsCollisionFilter.name = id;
				physicsCollisionFilter.me = getMaskFromFilter(id);
				physicsCollisionFilter.mask = mask;

				ArrayFn::pushBack(filterList, physicsCollisionFilter);
			}
		}

		uint32_t createNewFilterMask()
		{
			RESOURCE_COMPILER_ASSERT(filter != 0x80000000u
				, compileOptions
				, "Too many collision filters"
				);

			const uint32_t filterMask = filter;
			filter = filter << 1;
			return filterMask;
		}

		uint32_t getMaskFromFilter(StringId32 filter)
		{
			RESOURCE_COMPILER_ASSERT(MapFn::has(filterMap, filter)
				, compileOptions
				, "Filter not found"
				);

			return MapFn::get(filterMap, filter, 0u);
		}

		CompileOptions& compileOptions;
		Map<StringId32, uint32_t> filterMap;
		Array<PhysicsCollisionFilter> filterList;
		uint32_t filter = 1;
	};

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);
		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonRFn::parse(buffer, object);

		Array<PhysicsConfigMaterial> materials(getDefaultAllocator());
		Array<PhysicsConfigShape> shapes(getDefaultAllocator());
		Array<PhysicsConfigActor> actors(getDefaultAllocator());
		CollisionFilterCompiler collisionFilterCompiler(compileOptions);

		// Parse materials
		if (JsonObjectFn::has(object, "collisionFilters"))
		{
			collisionFilterCompiler.parse(object["collisionFilters"]);
		}
		if (JsonObjectFn::has(object, "materials"))
		{
			parseMaterials(object["materials"], materials);
		}
		if (JsonObjectFn::has(object, "shapes"))
		{
			parseShapes(object["shapes"], shapes);
		}
		if (JsonObjectFn::has(object, "actors"))
		{
			parseActors(object["actors"], actors);
		}

		// Setup struct for writing
		PhysicsConfigResource physicsConfigResource;
		physicsConfigResource.version = RESOURCE_VERSION_PHYSICS_CONFIG;
		physicsConfigResource.materialListCount = ArrayFn::getCount(materials);
		physicsConfigResource.shapeListCount = ArrayFn::getCount(shapes);
		physicsConfigResource.actorListCount = ArrayFn::getCount(actors);
		physicsConfigResource.filterListCount = ArrayFn::getCount(collisionFilterCompiler.filterList);

		uint32_t offset = sizeof(PhysicsConfigResource);
		physicsConfigResource.materialsOffset = offset;
		offset += sizeof(PhysicsConfigMaterial) * physicsConfigResource.materialListCount;

		physicsConfigResource.shapesOffset = offset;
		offset += sizeof(PhysicsConfigShape) * physicsConfigResource.shapeListCount;

		physicsConfigResource.actorsOffset = offset;
		offset += sizeof(PhysicsConfigActor) * physicsConfigResource.actorListCount;

		physicsConfigResource.filtersOffset = offset;
		offset += sizeof(PhysicsCollisionFilter) * physicsConfigResource.filterListCount;

		// Write all
		compileOptions.write(physicsConfigResource.version);
		compileOptions.write(physicsConfigResource.materialListCount);
		compileOptions.write(physicsConfigResource.materialsOffset);
		compileOptions.write(physicsConfigResource.shapeListCount);
		compileOptions.write(physicsConfigResource.shapesOffset);
		compileOptions.write(physicsConfigResource.actorListCount);
		compileOptions.write(physicsConfigResource.actorsOffset);
		compileOptions.write(physicsConfigResource.filterListCount);
		compileOptions.write(physicsConfigResource.filtersOffset);

		// Write material objects
		for (uint32_t i = 0; i < physicsConfigResource.materialListCount; ++i)
		{
			compileOptions.write(materials[i].name.id);
			compileOptions.write(materials[i].staticFriction);
			compileOptions.write(materials[i].dynamicFriction);
			compileOptions.write(materials[i].restitution);
		}

		// Write material objects
		for (uint32_t i = 0; i < physicsConfigResource.shapeListCount; ++i)
		{
			compileOptions.write(shapes[i].name.id);
			compileOptions.write(shapes[i].trigger);
			compileOptions.write(shapes[i]._pad[0]);
			compileOptions.write(shapes[i]._pad[1]);
			compileOptions.write(shapes[i]._pad[2]);
		}

		// Write actor objects
		for (uint32_t i = 0; i < physicsConfigResource.actorListCount; ++i)
		{
			compileOptions.write(actors[i].name.id);
			compileOptions.write(actors[i].linearDamping);
			compileOptions.write(actors[i].angularDamping);
			compileOptions.write(actors[i].flags);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(collisionFilterCompiler.filterList); ++i)
		{
			compileOptions.write(collisionFilterCompiler.filterList[i].name.id);
			compileOptions.write(collisionFilterCompiler.filterList[i].me);
			compileOptions.write(collisionFilterCompiler.filterList[i].mask);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t fileSize = file.getSize();
		void* resource = a.allocate(fileSize);
		file.read(resource, fileSize);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_PHYSICS_CONFIG, "Wrong version");
		return resource;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

} // namespace PhysicsConfigResourceInternalFn

namespace PhysicsConfigResourceFn
{
	const PhysicsConfigMaterial* getPhysicsConfigMaterial(const PhysicsConfigResource* physicsConfigResource, StringId32 name)
	{
		const PhysicsConfigMaterial* begin = (PhysicsConfigMaterial*)((const char*)physicsConfigResource + physicsConfigResource->materialsOffset);
		for (uint32_t i = 0; i < physicsConfigResource->materialListCount; ++i)
		{
			if (begin[i].name == name)
			{
				return &begin[i];
			}
		}

		RIO_FATAL("Material not found");
		return nullptr;
	}

	const PhysicsConfigShape* getPhysicsConfigShape(const PhysicsConfigResource* physicsConfigResource, StringId32 name)
	{
		const PhysicsConfigShape* begin = (PhysicsConfigShape*)((const char*)physicsConfigResource + physicsConfigResource->shapesOffset);
		for (uint32_t i = 0; i < physicsConfigResource->shapeListCount; ++i)
		{
			if (begin[i].name == name)
			{
				return &begin[i];
			}
		}

		RIO_FATAL("Shape not found");
		return nullptr;
	}

	const PhysicsConfigActor* getPhysicsConfigActor(const PhysicsConfigResource* physicsConfigResource, StringId32 name)
	{
		const PhysicsConfigActor* begin = (PhysicsConfigActor*)((const char*)physicsConfigResource + physicsConfigResource->actorsOffset);
		for (uint32_t i = 0; i < physicsConfigResource->actorListCount; ++i)
		{
			if (begin[i].name == name)
			{
				return &begin[i];
			}
		}

		RIO_FATAL("Actor not found");
		return nullptr;
	}

	const PhysicsCollisionFilter* getPhysicsCollisionFilter(const PhysicsConfigResource* physicsConfigResource, StringId32 name)
	{
		const PhysicsCollisionFilter* begin = (PhysicsCollisionFilter*)((const char*)physicsConfigResource + physicsConfigResource->filtersOffset);
		for (uint32_t i = 0; i < physicsConfigResource->filterListCount; ++i)
		{
			if (begin[i].name == name)
			{
				return &begin[i];
			}
		}

		RIO_FATAL("Filter not found");
		return nullptr;
	}

} // namespace PhysicsConfigResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka