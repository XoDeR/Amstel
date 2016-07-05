// Copyright (c) 2016 Volodymyr Syvochka
#include "Script/ScriptStack.h"

#include "Device/Device.h"
#include "Resource/ResourcePackage.h"

#include "World/DebugLine.h"
#include "World/Gui.h"
#include "World/Level.h"
#include "World/PhysicsWorld.h"
#include "World/RenderWorld.h"
#include "World/SceneGraph.h"
#include "World/SoundWorld.h"
#include "World/World.h"

#include "Script/ScriptEnvironment.h"

namespace Rio
{

bool ScriptStack::getIsVector3(int i)
{
	return getDevice()->getScriptEnvironment()->getIsVector3((Vector3*)lua_touserdata(scriptState, i));
}

bool ScriptStack::getIsQuaternion(int i)
{
	return getDevice()->getScriptEnvironment()->getIsQuaternion((Quaternion*)lua_touserdata(scriptState, i));
}

bool ScriptStack::getIsMatrix4x4(int i)
{
	return getDevice()->getScriptEnvironment()->getIsMatrix4x4((Matrix4x4*)lua_touserdata(scriptState, i));
}

void ScriptStack::checkTemporary(int i, const Vector3* vector3)
{
	ScriptEnvironment* scriptEnvironment = getDevice()->getScriptEnvironment();
	if (!getIsPointer(i) || !scriptEnvironment->getIsVector3(vector3))
	{
		luaL_typerror(scriptState, i, "Vector3");
	}
}

void ScriptStack::checkTemporary(int i, const Quaternion* quaternion)
{
	ScriptEnvironment* scriptEnvironment = getDevice()->getScriptEnvironment();
	if (!getIsPointer(i) || !scriptEnvironment->getIsQuaternion(quaternion))
	{
		luaL_typerror(scriptState, i, "Quaternion");
	}
}

void ScriptStack::checkTemporary(int i, const Matrix4x4* matrix4x4)
{
	ScriptEnvironment* scriptEnvironment = getDevice()->getScriptEnvironment();
	if (!getIsPointer(i) || !scriptEnvironment->getIsMatrix4x4(matrix4x4))
	{
		luaL_typerror(scriptState, i, "Matrix4x4");
	}
}

void ScriptStack::pushVector2(const Vector2& vector2)
{
	Vector3 vector3;
	vector3.x = vector2.x;
	vector3.y = vector2.y;
	vector3.z = 0.0f;
	pushVector3(vector3);
}

void ScriptStack::pushVector3(const Vector3& vector3)
{
	lua_pushlightuserdata(scriptState, getDevice()->getScriptEnvironment()->getNextVector3(vector3));
}

void ScriptStack::pushQuaternion(const Quaternion& quaternion)
{
	lua_pushlightuserdata(scriptState, getDevice()->getScriptEnvironment()->getNextQuaternion(quaternion));
}

void ScriptStack::pushMatrix4x4(const Matrix4x4& matrix4x4)
{
	lua_pushlightuserdata(scriptState, getDevice()->getScriptEnvironment()->getNextMatrix4x4(matrix4x4));
}

void ScriptStack::pushColor4(const Color4& color4)
{
	// Color4 represented as Quaternion
	Quaternion quaternion;
	quaternion.x = color4.x;
	quaternion.y = color4.y;
	quaternion.z = color4.z;
	quaternion.w = color4.w;
	pushQuaternion(quaternion);
}

void ScriptStack::checkType(int i, const DebugLine* debugLine)
{
	if (!getIsPointer(i) || *(uint32_t*)debugLine != DebugLine::MARKER)
	{
		luaL_typerror(scriptState, i, "DebugLine");
	}
}

void ScriptStack::checkType(int i, const ResourcePackage* resourcePackage)
{
	if (!getIsPointer(i) || *(uint32_t*)resourcePackage != ResourcePackage::MARKER)
	{
		luaL_typerror(scriptState, i, "ResourcePackage");
	}
}

void ScriptStack::checkType(int i, const World* world)
{
	if (!getIsPointer(i) || *(uint32_t*)world != World::MARKER)
	{
		luaL_typerror(scriptState, i, "World");
	}
}

void ScriptStack::checkType(int i, const SceneGraph* sceneGraph)
{
	if (!getIsPointer(i) || *(uint32_t*)sceneGraph != SceneGraph::MARKER)
	{
		luaL_typerror(scriptState, i, "SceneGraph");
	}
}

void ScriptStack::checkType(int i, const RenderWorld* renderWorld)
{
	if (!getIsPointer(i) || *(uint32_t*)renderWorld != RenderWorld::MARKER)
	{
		luaL_typerror(scriptState, i, "RenderWorld");
	}
}

void ScriptStack::checkType(int i, const Level* level)
{
	if (!getIsPointer(i) || *(uint32_t*)level != Level::MARKER)
	{
		luaL_typerror(scriptState, i, "Level");
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka