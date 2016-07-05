// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#include "Core/Error/Error.h"
#include "Device/Device.h"
#include "Device/Log.h"
#include "Resource/ScriptResource.h"
#include "Resource/ResourceManager.h"
#include "Script/ScriptEnvironment.h"
#include "Script/ScriptStack.h"

#include <stdarg.h>

namespace Rio
{

extern void loadApi(ScriptEnvironment& ScriptEnvironment);

// When an error occurs, logs the error message and pauses the engine
static int scriptErrorHandlerFunction(lua_State* scriptState)
{
	lua_getfield(scriptState, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(scriptState, -1))
	{
		lua_pop(scriptState, 1);
		return 0;
	}

	lua_getfield(scriptState, -1, "traceback");
	if (!lua_isfunction(scriptState, -1))
	{
		lua_pop(scriptState, 2);
		return 0;
	}

	lua_pushvalue(scriptState, 1); // Pass error message
	lua_pushinteger(scriptState, 2);
	lua_call(scriptState, 2, 1); // Call debug.traceback

	RIO_LOGE(lua_tostring(scriptState, -1)); // Print error message
	lua_pop(scriptState, 1); // Remove error message from stack
	lua_pop(scriptState, 1); // Remove debug.traceback from stack

	getDevice()->pause();
	return 0;
}

// Redirects require to the resource manager
static int require(lua_State* scriptState)
{
	using namespace ScriptResourceFn;
	ScriptStack stack(scriptState);
	const ScriptResource* scriptResource = (ScriptResource*)getDevice()->getResourceManager()->get(RESOURCE_TYPE_SCRIPT, stack.getResourceId(1));
	luaL_loadbuffer(scriptState, getProgram(scriptResource), scriptResource->size, "");
	return 1;
}

ScriptEnvironment::ScriptEnvironment()
{
	scriptState = luaL_newstate();
	RIO_ASSERT(scriptState, "Unable to create lua state");
}

ScriptEnvironment::~ScriptEnvironment()
{
	lua_close(scriptState);
}

void ScriptEnvironment::loadScriptLibraries()
{
	lua_gc(scriptState, LUA_GCSTOP, 0);

	// Open default libraries
	luaL_openlibs(scriptState);

	// Register Rio libraries
	loadApi(*this);

	// Register custom loader
	lua_getfield(scriptState, LUA_GLOBALSINDEX, "package");
	lua_getfield(scriptState, -1, "loaders");
	lua_remove(scriptState, -2);

	int loadersCount = 0;
	lua_pushnil(scriptState);
	while (lua_next(scriptState, -2) != 0)
	{
		lua_pop(scriptState, 1);
		loadersCount++;
	}
	lua_pushinteger(scriptState, loadersCount + 1);
	lua_pushcfunction(scriptState, require);
	lua_rawset(scriptState, -3);
	lua_pop(scriptState, 1);

	// Create metatable for lightuserdata
	lua_pushlightuserdata(scriptState, 0);
	lua_getfield(scriptState, LUA_REGISTRYINDEX, "Lightuserdata_mt");
	lua_setmetatable(scriptState, -2);
	lua_pop(scriptState, 1);

	// Ensure stack is clean
	RIO_ASSERT(lua_gettop(scriptState) == 0, "Stack not clean");

	lua_gc(scriptState, LUA_GCRESTART, 0);
}

void ScriptEnvironment::execute(const ScriptResource* scriptResource)
{
	using namespace ScriptResourceFn;
	lua_pushcfunction(scriptState, scriptErrorHandlerFunction);
	luaL_loadbuffer(scriptState, getProgram(scriptResource), scriptResource->size, "<unknown>");
	lua_pcall(scriptState, 0, 0, -2);
	lua_pop(scriptState, 1);
}

void ScriptEnvironment::executeScriptString(const char* scriptString)
{
	lua_pushcfunction(scriptState, scriptErrorHandlerFunction);
	luaL_loadstring(scriptState, scriptString);
	lua_pcall(scriptState, 0, 0, -2);
	lua_pop(scriptState, 1);
}

void ScriptEnvironment::addModuleFunction(const char* module, const char* name, const lua_CFunction function)
{
	luaL_newmetatable(scriptState, module);
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = function;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(scriptState, NULL, entry);
	lua_setglobal(scriptState, module);
	lua_pop(scriptState, -1);
}

void ScriptEnvironment::addModuleFunction(const char* module, const char* name, const char* function)
{
	luaL_newmetatable(scriptState, module);
	lua_getglobal(scriptState, function);
	lua_setfield(scriptState, -2, name);
	lua_setglobal(scriptState, module);
}

void ScriptEnvironment::setModuleConstructor(const char* module, const lua_CFunction function)
{
	// Create dummy tables to be used as module's metatable
	lua_createtable(scriptState, 0, 1);
	lua_pushstring(scriptState, "__call");
	lua_pushcfunction(scriptState, function);
	lua_settable(scriptState, 1); // dummy.__call = function
	lua_getglobal(scriptState, module);
	lua_pushvalue(scriptState, -2); // Duplicate dummy metatable
	lua_setmetatable(scriptState, -2); // setmetatable(module, dummy)
	lua_pop(scriptState, -1);
}

void ScriptEnvironment::callGlobalFunction(const char* function, uint8_t argumentListCount, ...)
{
	RIO_ASSERT_NOT_NULL(function);

	ScriptStack scriptStack(scriptState);

	va_list variableArgumentsList;
	va_start(variableArgumentsList, argumentListCount);

	lua_pushcfunction(scriptState, scriptErrorHandlerFunction);
	lua_getglobal(scriptState, function);

	for (uint8_t i = 0; i < argumentListCount; i++)
	{
		const int type = va_arg(variableArgumentsList, int);
		switch (type)
		{
			case ARGUMENT_FLOAT:
			{
				scriptStack.pushFloat(va_arg(variableArgumentsList, double));
				break;
			}
			default:
			{
				RIO_ASSERT(false, "Error, lua argument unknown");
				break;
			}
		}
	}

	va_end(variableArgumentsList);
	lua_pcall(scriptState, argumentListCount, 0, -argumentListCount - 2);
	lua_pop(scriptState, -1);
}

Vector3* ScriptEnvironment::getNextVector3(const Vector3& vector3)
{
	RIO_ASSERT(vector3ListUsedCount < RIO_MAX_LUA_VECTOR3, "Maximum number of Vector3 reached");
	return &(vector3Buffer[vector3ListUsedCount++] = vector3);
}

Quaternion* ScriptEnvironment::getNextQuaternion(const Quaternion& quaternion)
{
	RIO_ASSERT(quaternionListUsedCount < RIO_MAX_LUA_QUATERNION, "Maximum number of Quaternion reached");
	return &(quaternionBuffer[quaternionListUsedCount++] = quaternion);
}

Matrix4x4* ScriptEnvironment::getNextMatrix4x4(const Matrix4x4& matrix4x4)
{
	RIO_ASSERT(matrix4ListUsedCount < RIO_MAX_LUA_MATRIX4X4, "Maximum number of Matrix4x4 reached");
	return &(matrix4Buffer[matrix4ListUsedCount++] = matrix4x4);
}

bool ScriptEnvironment::getIsVector3(const Vector3* vector3) const
{
	return vector3 >= &vector3Buffer[0]
		&& vector3 <= &vector3Buffer[RIO_MAX_LUA_VECTOR3 - 1];
}

bool ScriptEnvironment::getIsQuaternion(const Quaternion* quaternion) const
{
	return quaternion >= &quaternionBuffer[0]
		&& quaternion <= &quaternionBuffer[RIO_MAX_LUA_QUATERNION - 1];
}

bool ScriptEnvironment::getIsMatrix4x4(const Matrix4x4* matrix4x4) const
{
	return matrix4x4 >= &matrix4Buffer[0]
		&& matrix4x4 <= &matrix4Buffer[RIO_MAX_LUA_MATRIX4X4 - 1];
}

void ScriptEnvironment::getTemporaryObjectsCount(uint32_t& vector3ListUsedCount, uint32_t& quaternionListUsedCount, uint32_t& matrix4ListUsedCount)
{
	vector3ListUsedCount = this->vector3ListUsedCount;
	quaternionListUsedCount = this->quaternionListUsedCount;
	matrix4ListUsedCount = this->matrix4ListUsedCount;
}

void ScriptEnvironment::setTemporaryObjectsCount(uint32_t vector3ListUsedCount, uint32_t quaternionListUsedCount, uint32_t matrix4ListUsedCount)
{
	this->vector3ListUsedCount = vector3ListUsedCount;
	this->quaternionListUsedCount = quaternionListUsedCount;
	this->matrix4ListUsedCount = matrix4ListUsedCount;
}

void ScriptEnvironment::resetTemporaryTypes()
{
	this->vector3ListUsedCount = 0;
	this->quaternionListUsedCount = 0;
	this->matrix4ListUsedCount = 0;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka