// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"
#include "Core/Base/Types.h"
#include "Core/Math/MathTypes.h"
#include "Resource/ResourceTypes.h"

#include <lua.hpp>

namespace Rio
{

enum LuaArgumentType
{
	ARGUMENT_FLOAT
};

// Wraps a subset of Lua functions and provides utilities for extending Lua
struct ScriptEnvironment
{
	ScriptEnvironment();
	~ScriptEnvironment();

	// Loads lua libraries
	void loadScriptLibraries();
	// Executes the lua script resource <scriptResource>
	void execute(const ScriptResource* scriptResource);
	// Executes the <script> string
	void executeScriptString(const char* scriptString);
	// Adds the function with the given <name> and <function> to the table <module>
	void addModuleFunction(const char* module, const char* name, const lua_CFunction function);
	// Adds the function with the given <name> and <function> to the table <module>
	void addModuleFunction(const char* module, const char* name, const char* function);
	// Sets the constructor for the table <module> to the given function
	void setModuleConstructor(const char* module, const lua_CFunction function);
	// Calls the global <function> with <argumentListCount>
	// Each argument is a pair (type, value)
	// Example:
	// callGlobalFunction("myFunction", 1, ARGUMENT_FLOAT, 3.14f)
	// Returns true if success, false otherwise
	void callGlobalFunction(const char* function, uint8_t argumentListCount, ...);
	// Returns the number of temporary objects in use
	void getTemporaryObjectsCount(uint32_t& vector3ListUsedCount, uint32_t& quaternionListUsedCount, uint32_t& matrix4ListUsedCount);
	// Sets the number of temporary objects in use
	void setTemporaryObjectsCount(uint32_t vector3ListUsedCount, uint32_t quaternionListUsedCount, uint32_t matrix4ListUsedCount);
	// Resets temporary types
	void resetTemporaryTypes();
	// Returns a new temporary Vector3
	Vector3* getNextVector3(const Vector3& vector3);
	// Returns a new temporary Quaternion
	Quaternion* getNextQuaternion(const Quaternion& quaternion);
	// Returns a new temporary Matrix4x4
	Matrix4x4* getNextMatrix4x4(const Matrix4x4& matrix4x4);
	// Returns whether <p> is a temporary Vector3
	bool getIsVector3(const Vector3* vector3) const;
	// Returns whether <p> is a temporary Quaternion
	bool getIsQuaternion(const Quaternion* quaternion) const;
	// Returns whether <p> is a temporary Matrix4x4
	bool getIsMatrix4x4(const Matrix4x4* matrix4x4) const;

	lua_State* scriptState = nullptr;
	uint32_t vector3ListUsedCount = 0;
	Vector3 vector3Buffer[RIO_MAX_LUA_VECTOR3];
	uint32_t quaternionListUsedCount = 0;
	Quaternion quaternionBuffer[RIO_MAX_LUA_QUATERNION];
	uint32_t matrix4ListUsedCount = 0;
	Matrix4x4 matrix4Buffer[RIO_MAX_LUA_MATRIX4X4];
private:
	// Disable copying
	ScriptEnvironment(const ScriptEnvironment&) = delete;
	ScriptEnvironment& operator=(const ScriptEnvironment&) = delete;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka