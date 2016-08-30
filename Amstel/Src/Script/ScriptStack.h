// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"
#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

#include <lua.hpp>

#if RIO_DEBUG
	#define LUA_ASSERT(condition, stack, msg, ...) do { if (!(condition)) {\
		stack.pushStringWithFormat("\nLua assertion failed: %s\n\t" msg "\n", #condition, ##__VA_ARGS__);\
		lua_error(stack.scriptState); }} while (0);
#else
	#define LUA_ASSERT(...) ((void)0)
#endif // RIO_DEBUG

#define LIGHTDATA_TYPE_BITS 2
#define LIGHTDATA_TYPE_MASK 0x3
#define LIGHTDATA_TYPE_SHIFT 0

#define POINTER_MARKER 0x0
#define UNIT_MARKER 0x1

namespace Rio
{

struct ScriptStack
{
	ScriptStack(lua_State* scriptState)
		: scriptState(scriptState)
	{
	}

	// Returns the number of elements in the stack
	// When called inside a function, it can be used to count
	// the number of arguments passed to the function itself
	int getArgumentsCount()
	{
		return lua_gettop(scriptState);
	}

	// Removes the element at the given valid index, shifting down the elements
	// above this index to fill the gap
	// Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position
	void remove(int i)
	{
		lua_remove(scriptState, i);
	}

	// Pops <n> elements from the stack
	void pop(int n)
	{
		lua_pop(scriptState, n);
	}

	bool getIsNil(int i)
	{
		return lua_isnil(scriptState, i) == 1;
	}

	bool getIsBool(int i)
	{
		return lua_isboolean(scriptState, i) == 1;
	}

	bool getIsNumber(int i)
	{
		return lua_isnumber(scriptState, i) == 1;
	}

	bool getIsString(int i)
	{
		return lua_isstring(scriptState, i) == 1;
	}

	bool getIsPointer(int i)
	{
		return lua_islightuserdata(scriptState, i) == 1
			&& ((uintptr_t)lua_touserdata(scriptState, i) & 0x3) == 0;
	}

	bool getIsFunction(int i)
	{
		return lua_isfunction(scriptState, i) == 1;
	}

	bool getIsTable(int i)
	{
		return lua_istable(scriptState, i) == 1;
	}

	bool getIsVector3(int i);
	bool getIsQuaternion(int i);
	bool getIsMatrix4x4(int i);

	// Wraps lua_type
	int getValueType(int i)
	{
		return lua_type(scriptState, i);
	}

	bool getBool(int i)
	{
		return lua_toboolean(scriptState, i) == 1;
	}

	int getInteger(int i)
	{
		return (int)lua_tonumber(scriptState, i);
	}

	float getFloat(int i)
	{
		return (float)lua_tonumber(scriptState, i);
	}

	const char* getString(int i)
	{
		return lua_tostring(scriptState, i);
	}

	void* getPointer(int i)
	{
		if (!lua_isuserdata(scriptState, i))
		{
			luaL_typerror(scriptState, i, "lightuserdata");
		}

		void* p = lua_touserdata(scriptState, i);
		RIO_ASSERT_NOT_NULL(p);
		return p;
	}

	uint32_t getId(int i)
	{
#if !RIO_RELEASE
		return (uint32_t)luaL_checknumber(scriptState, i);
#else
		return (uint32_t)lua_tonumber(scriptState, i);
#endif // !RIO_RELEASE
	}

	StringId32 getStringId32(int i)
	{
		return StringId32(getString(i));
	}

	StringId64 getStringId64(int i)
	{
		return StringId64(getString(i));
	}

	StringId64 getResourceId(int i)
	{
		return StringId64(getString(i));
	}

	DebugGui* getDebugGui(int i)
	{
		DebugGui* debugGui = (DebugGui*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, debugGui);
#endif // !RIO_RELEASE
		return debugGui;
	}

	DebugLine* getDebugLine(int i)
	{
		DebugLine* debugLine = (DebugLine*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, debugLine);
#endif // !RIO_RELEASE
		return debugLine;
	}

	ResourcePackage* getResourcePackage(int i)
	{
		ResourcePackage* resourcePackage = (ResourcePackage*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, resourcePackage);
#endif // !RIO_RELEASE
		return resourcePackage;
	}

	World* getWorld(int i)
	{
		World* world = (World*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, world);
#endif // !RIO_RELEASE
		return world;
	}

	SceneGraph* getSceneGraph(int i)
	{
		SceneGraph* sceneGraph = (SceneGraph*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, sceneGraph);
#endif // !RIO_RELEASE
		return sceneGraph;
	}

	Level* getLevel(int i)
	{
		Level* level = (Level*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, level);
#endif // !RIO_RELEASE
		return level;
	}

	RenderWorld* getRenderWorld(int i)
	{
		RenderWorld* renderWorld = (RenderWorld*)getPointer(i);
#if !RIO_RELEASE
		checkType(i, renderWorld);
#endif // !RIO_RELEASE
		return renderWorld;
	}

	PhysicsWorld* getPhysicsWorld(int i)
	{
		PhysicsWorld* physicsWorld = (PhysicsWorld*)getPointer(i);
#if !RIO_RELEASE
		// TODO
		//if (*(uint32_t*)physicsWorld != PHYSICS_WORLD_MARKER)
		//{
		//	luaL_typerror(scriptState, i, "PhysicsWorld");
		//}
#endif // !RIO_RELEASE
		return physicsWorld;
	}

	SoundWorld* getSoundWorld(int i)
	{
		SoundWorld* soundWorld = (SoundWorld*)getPointer(i);
#if !RIO_RELEASE
		// TODO
		//if (*(uint32_t*)soundWorld != SOUND_WORLD_MARKER)
		//{
		//	luaL_typerror(scriptState, i, "SoundWorld");
		//}
#endif // !RIO_RELEASE
		return soundWorld;
	}

	UnitId getUnit(int i)
	{
		uint32_t unitPointerWithExtraBits = (uint32_t)(uintptr_t)getPointer(i);
#if !RIO_RELEASE
		if ((unitPointerWithExtraBits & LIGHTDATA_TYPE_MASK) != UNIT_MARKER)
		{
			luaL_typerror(scriptState, i, "UnitId");
		}
#endif // !RIO_RELEASE
		UnitId id;
		id.index = unitPointerWithExtraBits >> 2; // 2 bits reserved
		return id;
	}

	CameraInstance getCamera(int i)
	{
		CameraInstance cameraInstance = { getId(i) };
		return cameraInstance;
	}

	TransformInstance getTransform(int i)
	{
		TransformInstance transformInstance = { getId(i) };
		return transformInstance;
	}

	MeshInstance getMeshInstance(int i)
	{
		MeshInstance meshInstance = { getId(i) };
		return meshInstance;
	}

	SpriteInstance getSpriteInstance(int i)
	{
		SpriteInstance spriteInstance = { getId(i) };
		return spriteInstance;
	}

	LightInstance getLightInstance(int i)
	{
		LightInstance lightInstance = { getId(i) };
		return lightInstance;
	}

	Material* getMaterial(int i)
	{
		return (Material*)getPointer(i);
	}

	ActorInstance getActor(int i)
	{
		ActorInstance actorInstance = { getId(i) };
		return actorInstance;
	}

	ControllerInstance getController(int i)
	{
		ControllerInstance controllerInstance = { getId(i) };
		return controllerInstance;
	}

	SoundInstanceId getSoundInstanceId(int i)
	{
		return getId(i);
	}

	Vector2 getVector2(int i)
	{
		Vector3 vector3 = getVector3(i);
		Vector2 vector2;
		vector2.x = vector3.x;
		vector2.y = vector3.y;
		return vector2;
	}

	Vector3& getVector3(int i)
	{
		Vector3* vector3 = (Vector3*)getPointer(i);
#if !RIO_RELEASE
		checkTemporary(i, vector3);
#endif // !RIO_RELEASE
		return *vector3;
	}

	Quaternion& getQuaternion(int i)
	{
		Quaternion* quaternion = (Quaternion*)getPointer(i);
#if !RIO_RELEASE
		checkTemporary(i, quaternion);
#endif // !RIO_RELEASE
		return *quaternion;
	}

	Matrix4x4& getMatrix4x4(int i)
	{
		Matrix4x4* matrix4x4 = (Matrix4x4*)getPointer(i);
#if !RIO_RELEASE
		checkTemporary(i, matrix4x4);
#endif // !RIO_RELEASE
		return *matrix4x4;
	}

	Color4 getColor4(int i)
	{
		Quaternion quaternion = getQuaternion(i);
		Color4 color4;
		color4.x = quaternion.x;
		color4.y = quaternion.y;
		color4.z = quaternion.z;
		color4.w = quaternion.w;
		return color4;
	}

	Vector2& getVector2Box(int i)
	{
		Vector2* vector2 = (Vector2*)luaL_checkudata(scriptState, i, "Vector2Box");
		return *vector2;
	}

	Vector3& getVector3Box(int i)
	{
		Vector3* vector3 = (Vector3*)luaL_checkudata(scriptState, i, "Vector3Box");
		return *vector3;
	}

	Quaternion& getQuaternionBox(int i)
	{
		Quaternion* quaternion = (Quaternion*)luaL_checkudata(scriptState, i, "QuaternionBox");
		return *quaternion;
	}

	Matrix4x4& getMatrix4x4Box(int i)
	{
		Matrix4x4* matrix4x4 = (Matrix4x4*)luaL_checkudata(scriptState, i, "Matrix4x4Box");
		return *matrix4x4;
	}

	void pushNil()
	{
		lua_pushnil(scriptState);
	}

	void pushBool(bool value)
	{
		lua_pushboolean(scriptState, value);
	}

	void pushInteger(int value)
	{
		lua_pushnumber(scriptState, value);
	}

	void pushFloat(float value)
	{
		lua_pushnumber(scriptState, value);
	}

	void pushString(const char* s)
	{
		lua_pushstring(scriptState, s);
	}

	void pushStringWithFormat(const char* format, ...)
	{
		va_list variableArgumentList;
		va_start(variableArgumentList, format);
		lua_pushvfstring(scriptState, format, variableArgumentList);
		va_end(variableArgumentList);
	}

	void pushStringWithLength(const char* str, uint32_t length)
	{
		lua_pushlstring(scriptState, str, length);
	}

	void pushStringId(StringId32 value)
	{
		lua_pushnumber(scriptState, value.id);
	}

	void pushPointer(void* p)
	{
		RIO_ASSERT_NOT_NULL(p);
		lua_pushlightuserdata(scriptState, p);
	}

	void pushFunction(lua_CFunction function)
	{
		lua_pushcfunction(scriptState, function);
	}

	void pushId(uint32_t value)
	{
		lua_pushnumber(scriptState, value);
	}

	// Pushes an empty table onto the stack
	// When you want to set keys on the table, you have to use ScriptStack::pushKeyBegin()
	// and ScriptStack::pushKeyEnd() as in the following example:
	// ScriptStack stack(scriptState)
	// stack.pushTable()
	// stack.pushKeyBegin("foo"); stack.push_foo(); stack.pushKeyEnd()
	// stack.pushKeyBegin("bar"); stack.push_bar(); stack.pushKeyEnd()
	// return 1;
	void pushTable(int arrayElementsCount = 0, int nonArrayElementsCount = 0)
	{
		lua_createtable(scriptState, arrayElementsCount, nonArrayElementsCount);
	}

	// See ScriptStack::pushTable()
	void pushKeyBegin(const char* key)
	{
		lua_pushstring(scriptState, key);
	}

	// See ScriptStack::pushTable()
	void pushKeyBegin(int i)
	{
		lua_pushnumber(scriptState, i);
	}

	// See ScriptStack::pushTable()
	void pushKeyEnd()
	{
		lua_settable(scriptState, -3);
	}

	int getNext(int i)
	{
		return lua_next(scriptState, i);
	}

	void pushDebugGui(DebugGui* debugGui)
	{
		pushPointer(debugGui);
	}

	void pushDebugLine(DebugLine* debugLine)
	{
		pushPointer(debugLine);
	}

	void pushResourcePackage(ResourcePackage* resourcePackage)
	{
		pushPointer(resourcePackage);
	}

	void pushWorld(World* world)
	{
		pushPointer(world);
	};

	void pushSceneGraph(SceneGraph* sceneGraph)
	{
		pushPointer(sceneGraph);
	}

	void pushLevel(Level* level)
	{
		pushPointer(level);
	}

	void pushRenderWorld(RenderWorld* renderWorld)
	{
		pushPointer(renderWorld);
	}

	void pushPhysicsWorld(PhysicsWorld* physicsWorld)
	{
		pushPointer(physicsWorld);
	}

	void pushSoundWorld(SoundWorld* soundWorld)
	{
		pushPointer(soundWorld);
	}

	void pushUnit(UnitId id)
	{
		uint32_t encoded = (id.index << 2) | UNIT_MARKER;
		pushPointer((void*)(uintptr_t)encoded);
	}

	void pushCamera(CameraInstance cameraInstance)
	{
		pushId(cameraInstance.i);
	}

	void pushTransform(TransformInstance transformInstance)
	{
		pushId(transformInstance.i);
	}

	void pushMeshInstance(MeshInstance meshInstance)
	{
		pushId(meshInstance.i);
	}

	void pushSpriteInstance(SpriteInstance spriteInstance)
	{
		pushId(spriteInstance.i);
	}

	void pushLightInstance(LightInstance lightInstance)
	{
		pushId(lightInstance.i);
	}

	void pushMaterial(Material* material)
	{
		pushPointer(material);
	}

	void pushActor(ActorInstance actorInstance)
	{
		pushId(actorInstance.i);
	}

	void pushController(ControllerInstance controllerInstance)
	{
		pushId(controllerInstance.i);
	}

	void pushSoundInstanceId(SoundInstanceId soundInstanceId)
	{
		pushId(soundInstanceId);
	}

	void pushVector2(const Vector2& vector2);
	void pushVector3(const Vector3& vector3);
	void pushMatrix4x4(const Matrix4x4& matrix4x4);
	void pushQuaternion(const Quaternion& quaternion);
	void pushColor4(const Color4& color4);

	void pushVector2Box(const Vector2& vector2)
	{
		Vector2* vector2ToPush = (Vector2*)lua_newuserdata(scriptState, sizeof(Vector2));
		luaL_getmetatable(scriptState, "Vector2Box");
		lua_setmetatable(scriptState, -2);
		*vector2ToPush = vector2;
	}

	void pushVector3Box(const Vector3& vector3)
	{
		Vector3* vector3ToPush = (Vector3*)lua_newuserdata(scriptState, sizeof(Vector3));
		luaL_getmetatable(scriptState, "Vector3Box");
		lua_setmetatable(scriptState, -2);
		*vector3ToPush = vector3;
	}

	void pushQuaternionBox(const Quaternion& quaternion)
	{
		Quaternion* quaternionToPush = (Quaternion*)lua_newuserdata(scriptState, sizeof(Quaternion));
		luaL_getmetatable(scriptState, "QuaternionBox");
		lua_setmetatable(scriptState, -2);
		*quaternionToPush = quaternion;
	}

	void pushMatrix4x4Box(const Matrix4x4& matrix4x4)
	{
		Matrix4x4* matrix4x4ToPush = (Matrix4x4*)lua_newuserdata(scriptState, sizeof(Matrix4x4));
		luaL_getmetatable(scriptState, "Matrix4x4Box");
		lua_setmetatable(scriptState, -2);
		*matrix4x4ToPush = matrix4x4;
	}

	void checkTemporary(int i, const Vector3* vector3);
	void checkTemporary(int i, const Quaternion* quaternion);
	void checkTemporary(int i, const Matrix4x4* matrix4x4);

	void checkType(int i, const DebugGui* debugGui);
	void checkType(int i, const DebugLine* debugLine);
	void checkType(int i, const ResourcePackage* resourcePackage);
	void checkType(int i, const World* world);
	void checkType(int i, const SceneGraph* sceneGraph);
	void checkType(int i, const RenderWorld* renderWorld);
	void checkType(int i, const Level* level);

	lua_State* scriptState;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka