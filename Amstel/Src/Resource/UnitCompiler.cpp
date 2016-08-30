// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/UnitCompiler.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/SortMap.h"
#include "Core/Base/Macros.h"
#include "Core/Containers/Map.h"
#include "Core/Math/MathUtils.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Memory/TempAllocator.h"

#include "Resource/CompileOptions.h"
#include "Resource/UnitResource.h"
#include "Resource/PhysicsResource.h"

#include "World/WorldTypes.h"

namespace Rio
{

struct ProjectionInfo
{
	const char* name;
	ProjectionType::Enum type;
};

static const ProjectionInfo projectionInfoMap[] =
{
	{ "perspective",  ProjectionType::PERSPECTIVE  },
	{ "orthographic", ProjectionType::ORTHOGRAPHIC }
};
RIO_STATIC_ASSERT(RIO_COUNTOF(projectionInfoMap) == ProjectionType::COUNT);

struct LightInfo
{
	const char* name;
	LightType::Enum type;
};

static const LightInfo lightInfoMap[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
RIO_STATIC_ASSERT(RIO_COUNTOF(lightInfoMap) == LightType::COUNT);

static ProjectionType::Enum getProjectionTypeFromName(const char* name)
{
	for (uint32_t i = 0; i < RIO_COUNTOF(projectionInfoMap); ++i)
	{
		if (strcmp(name, projectionInfoMap[i].name) == 0)
		{
			return projectionInfoMap[i].type;
		}
	}

	return ProjectionType::COUNT;
}

static LightType::Enum getLightTypeFromName(const char* name)
{
	for (uint32_t i = 0; i < RIO_COUNTOF(lightInfoMap); ++i)
	{
		if (strcmp(name, lightInfoMap[i].name) == 0)
		{
			return lightInfoMap[i].type;
		}
	}

	return LightType::COUNT;
}

static Buffer compileTransform(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	TransformDesc transformDesc;
	transformDesc.position = JsonRFn::parseVector3(jsonObject["position"]);
	transformDesc.rotation = JsonRFn::parseQuaternion(jsonObject["rotation"]);
	transformDesc.scale = JsonRFn::parseVector3(jsonObject["scale"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&transformDesc, sizeof(transformDesc));
	return buffer;
}

static Buffer compileCamera(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	DynamicString type(ta);
	JsonRFn::parseString(jsonObject["projection"], type);

	ProjectionType::Enum projectionType = getProjectionTypeFromName(type.getCStr());
	RESOURCE_COMPILER_ASSERT(projectionType != ProjectionType::COUNT
		, compileOptions
		, "Unknown projection type: '%s'"
		, type.getCStr()
		);

	CameraDesc cameraDesc;
	cameraDesc.type = projectionType;
	cameraDesc.fov = JsonRFn::parseFloat(jsonObject["fov"]);
	cameraDesc.nearRange = JsonRFn::parseFloat(jsonObject["nearRange"]);
	cameraDesc.farRange = JsonRFn::parseFloat(jsonObject["farRange"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&cameraDesc, sizeof(cameraDesc));
	return buffer;
}

static Buffer compileMeshRenderer(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	DynamicString meshResourceName(ta);
	JsonRFn::parseString(jsonObject["meshResource"], meshResourceName);
	RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_MESH, meshResourceName.getCStr(), compileOptions);

	MeshRendererDesc meshRendererDesc;
	meshRendererDesc.meshResource = JsonRFn::parseResourceId(jsonObject["meshResource"]);
	meshRendererDesc.geometryName = JsonRFn::parseStringId(jsonObject["geometryName"]);
	meshRendererDesc.materialResource = JsonRFn::parseResourceId(jsonObject["material"]);
	meshRendererDesc.visible = JsonRFn::parseBool(jsonObject["visible"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&meshRendererDesc, sizeof(meshRendererDesc));
	return buffer;
}

static Buffer compileSpriteRenderer(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	DynamicString spriteResource(ta);
	JsonRFn::parseString(jsonObject["spriteResource"], spriteResource);
	RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_SPRITE, spriteResource.getCStr(), compileOptions);

	SpriteRendererDesc spriteRendererDesc;
	spriteRendererDesc.spriteResourceName = JsonRFn::parseResourceId(jsonObject["spriteResource"]);
	spriteRendererDesc.materialResource = JsonRFn::parseResourceId(jsonObject["material"]);
	spriteRendererDesc.visible = JsonRFn::parseBool(jsonObject["visible"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&spriteRendererDesc, sizeof(spriteRendererDesc));
	return buffer;
}

static Buffer compileLight(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	DynamicString type(ta);
	JsonRFn::parseString(jsonObject["type"], type);

	LightType::Enum lightType = getLightTypeFromName(type.getCStr());
	RESOURCE_COMPILER_ASSERT(lightType != LightType::COUNT
		, compileOptions
		, "Unknown light type: '%s'"
		, type.getCStr()
		);

	LightDesc lightDesc;
	lightDesc.type = lightType;
	lightDesc.range = JsonRFn::parseFloat(jsonObject["range"]);
	lightDesc.intensity = JsonRFn::parseFloat(jsonObject["intensity"]);
	lightDesc.spotAngle = JsonRFn::parseFloat(jsonObject["spotAngle"]);
	lightDesc.color = JsonRFn::parseVector3(jsonObject["color"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&lightDesc, sizeof(lightDesc));
	return buffer;
}

UnitCompiler::UnitCompiler(CompileOptions& compileOptions)
	: compileOptions(compileOptions)
	, componentTypeDataMap(getDefaultAllocator())
	, componentTypeInfoArray(getDefaultAllocator())
{
	registerComponentCompiler("transform", &compileTransform, 0.0f);
	registerComponentCompiler("camera", &compileCamera, 1.0f);
	registerComponentCompiler("meshRenderer", &compileMeshRenderer, 1.0f);
	registerComponentCompiler("spriteRenderer", &compileSpriteRenderer, 1.0f);
	registerComponentCompiler("light", &compileLight, 1.0f);
	registerComponentCompiler("controller", &PhysicsResourceInternalFn::compileController, 1.0f);
	registerComponentCompiler("collider", &PhysicsResourceInternalFn::compileCollider, 1.0f);
	registerComponentCompiler("actor", &PhysicsResourceInternalFn::compileActor, 2.0f);
	registerComponentCompiler("joint", &PhysicsResourceInternalFn::compileJoint, 3.0f);
}

Buffer UnitCompiler::readUnit(const char* path)
{
	Buffer buffer = compileOptions.read(path);
	ArrayFn::pushBack(buffer, '\0');
	return buffer;
}

void UnitCompiler::compileUnit(const char* path)
{
	compileUnitFromJson(ArrayFn::begin(readUnit(path)));
}

void UnitCompiler::compileUnitFromJson(const char* json)
{
	Buffer data(getDefaultAllocator());
	ArrayFn::reserve(data, 1024*1024);

	uint32_t prefabsCount = 1;

	TempAllocator4096 ta;
	JsonObject prefabs[4] = { JsonObject(ta), JsonObject(ta), JsonObject(ta), JsonObject(ta) };
	JsonRFn::parse(json, prefabs[0]);

	for (uint32_t i = 0; i < RIO_COUNTOF(prefabs); ++i, ++prefabsCount)
	{
		const JsonObject& prefab = prefabs[i];

		if (!JsonObjectFn::has(prefab, "prefab"))
		{
			break;
		}

		TempAllocator512 ta;
		DynamicString path(ta);
		JsonRFn::parseString(prefab["prefab"], path);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_UNIT, path.getCStr(), compileOptions);
		path += "." RESOURCE_EXTENSION_UNIT;

		Buffer buffer = readUnit(path.getCStr());
		const char* d = ArrayFn::end(data);
		ArrayFn::push(data, ArrayFn::begin(buffer), ArrayFn::getCount(buffer));
		JsonRFn::parse(d, prefabs[i + 1]);
	}

	JsonObject& prefabRootJsonObject = prefabs[prefabsCount - 1];
	JsonObject prefabRootComponentList(ta);
	JsonRFn::parse(prefabRootJsonObject["components"], prefabRootComponentList);

	if (prefabsCount > 1)
	{
		// Merge prefabs' components
		for (uint32_t i = 0; i < prefabsCount; ++i)
		{
			const JsonObject& prefab = prefabs[prefabsCount - i - 1];

			if (!JsonObjectFn::has(prefab, "modifiedComponentList"))
			{
				continue;
			}

			JsonObject modifiedComponentList(ta);
			JsonRFn::parse(prefab["modifiedComponentList"], modifiedComponentList);

			auto begin = JsonObjectFn::begin(modifiedComponentList);
			auto end = JsonObjectFn::end(modifiedComponentList);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const FixedString id(&key.getData()[1], key.getLength()-1);
				const char* value = begin->pair.second;

				// TODO
				MapFn::remove(prefabRootComponentList.map, id);
				MapFn::set(prefabRootComponentList.map, id, value);
			}
		}
	}

	if (JsonObjectFn::getCount(prefabRootComponentList) > 0)
	{
		auto begin = JsonObjectFn::begin(prefabRootComponentList);
		auto end = JsonObjectFn::end(prefabRootComponentList);
		for (; begin != end; ++begin)
		{
			const char* value = begin->pair.second;

			TempAllocator512 ta;
			JsonObject component(ta);
			JsonRFn::parse(value, component);

			const StringId32 type = JsonRFn::parseStringId(component["type"]);

			Buffer buffer = compileComponent(type, component["data"]);
			addComponentData(type, buffer, unitListCount);
		}
	}

	++unitListCount;
}

void UnitCompiler::compileMultipleUnits(const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	auto begin = JsonObjectFn::begin(jsonObject);
	auto end = JsonObjectFn::end(jsonObject);

	for (; begin != end; ++begin)
	{
		compileUnitFromJson(begin->pair.second);
	}
}

Buffer UnitCompiler::getBlob()
{
	UnitResource unitResource;
	unitResource.version = RESOURCE_VERSION_UNIT;
	unitResource.unitListCount = this->unitListCount;
	unitResource.componentTypesCount = 0;

	auto begin = SortMapFn::begin(componentTypeDataMap);
	auto end = SortMapFn::end(componentTypeDataMap);

	for (; begin != end; ++begin)
	{
		const uint32_t componentDataCount = begin->pair.second.componentDataCount;
		if (componentDataCount > 0)
		{
			++unitResource.componentTypesCount;
		}
	}

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&unitResource, sizeof(unitResource));

	for (uint32_t i = 0; i < ArrayFn::getCount(componentTypeInfoArray); ++i)
	{
		const StringId32 type = componentTypeInfoArray[i].unitComponentTypeName;
		const ComponentTypeData& componentTypeData = SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator()));

		const Buffer& data = componentTypeData.data;
		const Array<uint32_t>& unitIndexList = componentTypeData.unitIndexList;
		const uint32_t componentDataListCount = componentTypeData.componentDataCount;

		if (componentDataListCount > 0)
		{
			ComponentData componentData;
			componentData.type = type;
			componentData.instancesCount = componentDataListCount;
			componentData.size = ArrayFn::getCount(data) + sizeof(uint32_t)*ArrayFn::getCount(unitIndexList);

			ArrayFn::push(buffer, (char*)&componentData, sizeof(componentData));
			ArrayFn::push(buffer, (char*)ArrayFn::begin(unitIndexList), sizeof(uint32_t)*ArrayFn::getCount(unitIndexList));
			ArrayFn::push(buffer, ArrayFn::begin(data), ArrayFn::getCount(data));
		}
	}

	return buffer;
}

void UnitCompiler::addComponentData(StringId32 type, const Buffer& data, uint32_t unitIndex)
{
	ComponentTypeData& componentTypeData = const_cast<ComponentTypeData&>(SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator())));

	ArrayFn::push(componentTypeData.data, ArrayFn::begin(data), ArrayFn::getCount(data));
	ArrayFn::pushBack(componentTypeData.unitIndexList, unitIndex);
	++componentTypeData.componentDataCount;
}

void UnitCompiler::registerComponentCompiler(const char* type, CompileFunction compileFunction, float spawnOrder)
{
	registerComponentCompiler(StringId32(type), compileFunction, spawnOrder);
}

void UnitCompiler::registerComponentCompiler(StringId32 type, CompileFunction compileFunction, float spawnOrder)
{
	ComponentTypeData componentTypeData(getDefaultAllocator());
	componentTypeData.compileFunction = compileFunction;

	ComponentTypeInfo componentTypeInfo;
	componentTypeInfo.unitComponentTypeName = type;
	componentTypeInfo.spawnOrder = spawnOrder;

	SortMapFn::set(componentTypeDataMap, type, componentTypeData);
	SortMapFn::sort(componentTypeDataMap);

	ArrayFn::pushBack(componentTypeInfoArray, componentTypeInfo);
	std::sort(ArrayFn::begin(componentTypeInfoArray), ArrayFn::end(componentTypeInfoArray));
}

Buffer UnitCompiler::compileComponent(StringId32 type, const char* json)
{
	RESOURCE_COMPILER_ASSERT(SortMapFn::has(componentTypeDataMap, type), compileOptions, "Unknown component");
	return SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator())).compileFunction(json, compileOptions);
}

} // namespace Rio
  // Copyright (c) 2016 Volodymyr Syvochka