// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Strings/StringStream.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Base/Guid.h"
#include "Core/Math/Color4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/MathTypes.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"

#include "Device/ConsoleServer.h"
#include "Device/Device.h"
#include "Device/InputDevice.h"
#include "Device/InputManager.h"
#include "Device/Profiler.h"

#include "Resource/ResourceManager.h"
#include "Resource/ResourcePackage.h"

#include "World/DebugLine.h"
#include "World/Gui.h"
#include "World/Material.h"
#include "World/PhysicsWorld.h"
#include "World/RenderWorld.h"
#include "World/SceneGraph.h"
#include "World/SoundWorld.h"
#include "World/UnitManager.h"
#include "World/World.h"

#include "Script/ScriptEnvironment.h"
#include "Script/ScriptStack.h"

namespace Rio
{

struct LightInfo
{
	const char* name;
	LightType::Enum type;
};

static LightInfo lightInfoMap[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni", LightType::OMNI },
	{ "spot", LightType::SPOT }
};
RIO_STATIC_ASSERT(RIO_COUNTOF(lightInfoMap) == LightType::COUNT);

struct ProjectionInfo
{
	const char* name;
	ProjectionType::Enum type;
};

static ProjectionInfo projectionInfoMap[] =
{
	{ "orthographic", ProjectionType::ORTHOGRAPHIC },
	{ "perspective",  ProjectionType::PERSPECTIVE  }
};
RIO_STATIC_ASSERT(RIO_COUNTOF(projectionInfoMap) == ProjectionType::COUNT);

struct RaycastInfo
{
	const char* name;
	RaycastMode::Enum mode;
};

static RaycastInfo raycastInfoMap[] =
{
	{ "closest", RaycastMode::CLOSEST },
	{ "all", RaycastMode::ALL }
};
RIO_STATIC_ASSERT(RIO_COUNTOF(raycastInfoMap) == RaycastMode::COUNT);

static LightType::Enum getLightTypeFromName(const char* name)
{
	for (uint32_t i = 0; i < RIO_COUNTOF(lightInfoMap); ++i)
	{
		if (strcmp(lightInfoMap[i].name, name) == 0)
		{
			return lightInfoMap[i].type;
		}
	}

	return LightType::COUNT;
}

static ProjectionType::Enum getProjectionTypeFromName(const char* name)
{
	for (uint32_t i = 0; i < RIO_COUNTOF(projectionInfoMap); ++i)
	{
		if (strcmp(projectionInfoMap[i].name, name) == 0)
		{
			return projectionInfoMap[i].type;
		}
	}

	return ProjectionType::COUNT;
}

static RaycastMode::Enum getRaycastModeFromName(const char* name)
{
	for (uint32_t i = 0; i < RIO_COUNTOF(raycastInfoMap); ++i)
	{
		if (strcmp(raycastInfoMap[i].name, name) == 0)
		{
			return raycastInfoMap[i].mode;
		}
	}

	return RaycastMode::COUNT;
}

static int math_getRayPlaneIntersection(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Plane plane = PlaneFn::createPlaneFromPointAndNormal(scriptStack.getVector3(3)
		, scriptStack.getVector3(4)
		);
	const float result = getRayPlaneIntersection(scriptStack.getVector3(1)
		, scriptStack.getVector3(2)
		, plane
		);
	scriptStack.pushFloat(result);
	return 1;
}

static int math_getRayDiscIntersection(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const float result = getRayDiscIntersection(scriptStack.getVector3(1)
		, scriptStack.getVector3(2)
		, scriptStack.getVector3(3)
		, scriptStack.getFloat(4)
		, scriptStack.getVector3(5)
		);
	scriptStack.pushFloat(result);
	return 1;
}

static int math_getRaySphereIntersection(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Sphere sphere;
	sphere.c = scriptStack.getVector3(3);
	sphere.r = scriptStack.getFloat(4);
	const float result = getRaySphereIntersection(scriptStack.getVector3(1)
		, scriptStack.getVector3(2)
		, sphere
		);
	scriptStack.pushFloat(result);
	return 1;
}

static int math_getRayObbIntersection(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const float result = getRayObbIntersection(scriptStack.getVector3(1)
		, scriptStack.getVector3(2)
		, scriptStack.getMatrix4x4(3)
		, scriptStack.getVector3(4)
		);
	scriptStack.pushFloat(result);
	return 1;
}

static int math_getRayTriangleIntersection(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const float result = getRayTriangleIntersection(scriptStack.getVector3(1)
		, scriptStack.getVector3(2)
		, scriptStack.getVector3(3)
		, scriptStack.getVector3(4)
		, scriptStack.getVector3(5)
		);
	scriptStack.pushFloat(result);
	return 1;
}

// Vector3
static int vector3_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector3 vector3;
	vector3.x = scriptStack.getFloat(1 + 1);
	vector3.y = scriptStack.getFloat(2 + 1);
	vector3.z = scriptStack.getFloat(3 + 1);
	scriptStack.pushVector3(vector3);
	return 1;
}

static int vector3_getX(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getVector3(1).x);
	return 1;
}

static int vector3_getY(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getVector3(1).y);
	return 1;
}

static int vector3_getZ(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getVector3(1).z);
	return 1;
}

static int vector3_setX(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getVector3(1).x = scriptStack.getFloat(2);
	return 0;
}

static int vector3_setY(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getVector3(1).y = scriptStack.getFloat(2);
	return 0;
}

static int vector3_setZ(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getVector3(1).z = scriptStack.getFloat(2);
	return 0;
}

static int vector3_getElements(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector3& a = scriptStack.getVector3(1);
	scriptStack.pushFloat(a.x);
	scriptStack.pushFloat(a.y);
	scriptStack.pushFloat(a.z);
	return 3;
}

static int vector3_add(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getVector3(1) + scriptStack.getVector3(2));
	return 1;
}

static int vector3_subtract(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getVector3(1) - scriptStack.getVector3(2));
	return 1;
}

static int vector3_multiply(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getVector3(1) * scriptStack.getFloat(2));
	return 1;
}

static int vector3_dot(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(dot(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_cross(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(cross(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_getAreEqual(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getVector3(1) == scriptStack.getVector3(2));
	return 1;
}

static int vector3_getLength(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getLength(scriptStack.getVector3(1)));
	return 1;
}

static int vector3_getLengthSquared(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getLengthSquared(scriptStack.getVector3(1)));
	return 1;
}

static int vector3_setLength(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setLength(scriptStack.getVector3(1), scriptStack.getFloat(2));
	return 0;
}

static int vector3_normalize(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(normalize(scriptStack.getVector3(1)));
	return 1;
}

static int vector3_getDistance(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getDistance(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_getDistanceSquared(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getDistanceSquared(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_getAngle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getAngle(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_max(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(max(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_min(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(min(scriptStack.getVector3(1), scriptStack.getVector3(2)));
	return 1;
}

static int vector3_getLinearInterpolation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getLinearInterpolation(scriptStack.getVector3(1), scriptStack.getVector3(2), scriptStack.getFloat(3)));
	return 1;
}

static int vector3_createForward(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_FORWARD);
	return 1;
}

static int vector3_createBackward(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_BACKWARD);
	return 1;
}

static int vector3_createLeft(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_LEFT);
	return 1;
}

static int vector3_createRight(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_RIGHT);
	return 1;
}

static int vector3_createUp(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_UP);
	return 1;
}

static int vector3_createDown(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_DOWN);
	return 1;
}

static int vector3_createZero(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(VECTOR3_ZERO);
	return 1;
}

static int vector3_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Vector3 v = scriptStack.getVector3(1);
	char buffer[256];
	snPrintF(buffer, sizeof(buffer), "%.4f %.4f %.4f", v.x, v.y, v.z);
	scriptStack.pushString(buffer);
	return 1;
}

static int vector2_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector2 vector2;
	vector2.x = scriptStack.getFloat(1 + 1);
	vector2.y = scriptStack.getFloat(2 + 1);
	scriptStack.pushVector2(vector2);
	return 1;
}

static int vector3box_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	if (scriptStack.getArgumentsCount() == 0 + 1)
	{
		scriptStack.pushVector3Box(VECTOR3_ZERO);
	}
	else if (scriptStack.getArgumentsCount() == 1 + 1)
	{
		scriptStack.pushVector3Box(scriptStack.getVector3(1 + 1));
	}
	else
	{
		Vector3 vector3;
		vector3.x = scriptStack.getFloat(1 + 1);
		vector3.y = scriptStack.getFloat(2 + 1);
		vector3.z = scriptStack.getFloat(3 + 1);
		scriptStack.pushVector3Box(vector3);
	}

	return 1;
}

static int vector3box_store(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	Vector3& vector3 = scriptStack.getVector3Box(1);

	if (scriptStack.getArgumentsCount() == 2)
	{
		vector3 = scriptStack.getVector3(2);
	}
	else
	{
		vector3 = createVector3(scriptStack.getFloat(2)
			, scriptStack.getFloat(3)
			, scriptStack.getFloat(4));
	}
	return 0;
}

static int vector3box_unbox(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getVector3Box(1));
	return 1;
}

static int vector3box_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector3& vector3 = scriptStack.getVector3Box(1);
	scriptStack.pushStringWithFormat("Vector3Box (%p)", &vector3);
	return 1;
}

// Matrix4x4
static int matrix4x4_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Matrix4x4 m;
	m.x.x = scriptStack.getFloat( 1 + 1);
	m.x.y = scriptStack.getFloat( 2 + 1);
	m.x.z = scriptStack.getFloat( 3 + 1);
	m.x.w = scriptStack.getFloat( 4 + 1);
	m.y.x = scriptStack.getFloat( 5 + 1);
	m.y.y = scriptStack.getFloat( 6 + 1);
	m.y.z = scriptStack.getFloat( 7 + 1);
	m.y.w = scriptStack.getFloat( 8 + 1);
	m.z.x = scriptStack.getFloat( 9 + 1);
	m.z.y = scriptStack.getFloat(10 + 1);
	m.z.z = scriptStack.getFloat(11 + 1);
	m.z.w = scriptStack.getFloat(12 + 1);
	m.t.x = scriptStack.getFloat(13 + 1);
	m.t.y = scriptStack.getFloat(14 + 1);
	m.t.z = scriptStack.getFloat(15 + 1);
	m.t.w = scriptStack.getFloat(16 + 1);
	scriptStack.pushMatrix4x4(m);
	return 1;
}

static int matrix4x4_createFromQuaternion(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(createMatrix4x4(scriptStack.getQuaternion(1), VECTOR3_ZERO));
	return 1;
}

static int matrix4x4_createFromTranslation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(createMatrix4x4(scriptStack.getVector3(1)));
	return 1;
}

static int matrix4x4_createFromQuaternionTranslation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(createMatrix4x4(scriptStack.getQuaternion(1), scriptStack.getVector3(2)));
	return 1;
}

static int matrix4x4_createFromAxes(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(createMatrix4x4(scriptStack.getVector3(1), scriptStack.getVector3(2), scriptStack.getVector3(3), scriptStack.getVector3(4)));
	return 1;
}

static int matrix4x4_copy(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getMatrix4x4(1));
	return 1;
}

static int matrix4x4_add(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getMatrix4x4(1) + scriptStack.getMatrix4x4(2));
	return 1;
}

static int matrix4x4_subtract(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getMatrix4x4(1) - scriptStack.getMatrix4x4(2));
	return 1;
}

static int matrix4x4_multiply(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getMatrix4x4(1) * scriptStack.getMatrix4x4(2));
	return 1;
}

static int matrix4x4_transpose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(transpose(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_getDeterminant(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getDeterminant(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_invert(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(invert(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_getAxisX(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getAxisX(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_getAxisY(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getAxisY(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_getAxisZ(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getAxisZ(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_setAxisX(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setAxisX(scriptStack.getMatrix4x4(1), scriptStack.getVector3(2));
	return 0;
}

static int matrix4x4_setAxisY(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setAxisY(scriptStack.getMatrix4x4(1), scriptStack.getVector3(2));
	return 0;
}

static int matrix4x4_setAxisZ(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setAxisZ(scriptStack.getMatrix4x4(1), scriptStack.getVector3(2));
	return 0;
}

static int matrix4x4_getTranslation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getTranslation(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_setTranslation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setTranslation(scriptStack.getMatrix4x4(1), scriptStack.getVector3(2));
	return 0;
}

static int matrix4x4_getRotationAsQuaternion(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(getRotationAsQuaternion(scriptStack.getMatrix4x4(1)));
	return 1;
}

static int matrix4x4_setRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	setRotation(scriptStack.getMatrix4x4(1), scriptStack.getQuaternion(2));
	return 0;
}

static int matrix4x4_createIdentity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(MATRIX4X4_IDENTITY);
	return 1;
}

static int matrix4x4_getTransform(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getVector3(2) * scriptStack.getMatrix4x4(1));
	return 1;
}

static int matrix4x4_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Matrix4x4& matrix4x4 = scriptStack.getMatrix4x4(1);
	char buffer[1024];
	snPrintF(buffer, sizeof(buffer),
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f"
		, matrix4x4.x.x, matrix4x4.x.y, matrix4x4.x.z, matrix4x4.y.w
		, matrix4x4.y.x, matrix4x4.y.y, matrix4x4.y.z, matrix4x4.y.w
		, matrix4x4.z.x, matrix4x4.z.y, matrix4x4.z.z, matrix4x4.z.w
		, matrix4x4.t.x, matrix4x4.t.y, matrix4x4.t.z, matrix4x4.t.w
		);
	scriptStack.pushString(buffer);
	return 1;
}

static int matrix4x4Box_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	if (scriptStack.getArgumentsCount() == 0 + 1)
	{
		scriptStack.pushMatrix4x4(MATRIX4X4_IDENTITY);
	}
	else
	{
		scriptStack.pushMatrix4x4Box(scriptStack.getMatrix4x4(1 + 1));
	}

	return 1;
}

static int matrix4x4Box_store(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getMatrix4x4Box(1) = scriptStack.getMatrix4x4(2);
	return 0;
}

static int matrix4x4Box_unbox(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getMatrix4x4Box(1));
	return 1;
}

static int matrix4x4Box_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Matrix4x4& matrix4x4 = scriptStack.getMatrix4x4Box(1);
	scriptStack.pushStringWithFormat("Matrix4x4Box (%p)", &matrix4x4);
	return 1;
}

// Quaternion
static int quaternion_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(createQuaternion(scriptStack.getVector3(1 + 1), scriptStack.getFloat(2 + 1)));
	return 1;
}

static int quaternion_createFromAxisAngle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(createQuaternion(scriptStack.getVector3(1), scriptStack.getFloat(2)));
	return 1;
}

static int quaternion_createFromElements(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(createQuaternion(scriptStack.getFloat(1), scriptStack.getFloat(2), scriptStack.getFloat(3), scriptStack.getFloat(4)));
	return 1;
}

static int quaternion_negate(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(-scriptStack.getQuaternion(1));
	return 1;
}

static int quaternion_createIdentity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(QUATERNION_IDENTITY);
	return 1;
}

static int quaternion_dot(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(dot(scriptStack.getQuaternion(1), scriptStack.getQuaternion(2)));
	return 1;
}

static int quaternion_getLength(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getLength(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(normalize(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_getConjugate(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(getConjugate(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_getInverse(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(getInverse(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_multiply(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(scriptStack.getQuaternion(1) * scriptStack.getQuaternion(2));
	return 1;
}

static int quaternion_multiplyByScalar(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(scriptStack.getQuaternion(1) * scriptStack.getFloat(2));
	return 1;
}

static int quaternion_getPower(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(getPower(scriptStack.getQuaternion(1), scriptStack.getFloat(2)));
	return 1;
}

static int quaternion_getElementList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Quaternion& quaternion = scriptStack.getQuaternion(1);
	scriptStack.pushFloat(quaternion.x);
	scriptStack.pushFloat(quaternion.y);
	scriptStack.pushFloat(quaternion.z);
	scriptStack.pushFloat(quaternion.w);
	return 4;
}

static int quaternion_createLook(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Vector3 up = scriptStack.getArgumentsCount() == 2 ? scriptStack.getVector3(2) : VECTOR3_YAXIS;
	scriptStack.pushQuaternion(getLook(scriptStack.getVector3(1), up));
	return 1;
}

static int quaternion_createRight(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getRight(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_createUp(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getUp(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_createForward(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(getForward(scriptStack.getQuaternion(1)));
	return 1;
}

static int quaternion_getLinearInterpolation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(getLinearInterpolation(scriptStack.getQuaternion(1), scriptStack.getQuaternion(2), scriptStack.getFloat(3)));
	return 1;
}

static int quaternion_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Quaternion quaternion = scriptStack.getQuaternion(1);
	char buffer[256];
	snPrintF(buffer, sizeof(buffer), "%.4f %.4f %.4f %.4f", quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	scriptStack.pushString(buffer);
	return 1;
}

static int quaternionBox_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	if (scriptStack.getArgumentsCount() == 0 + 1)
	{
		scriptStack.pushQuaternionBox(QUATERNION_IDENTITY);
	}
	else if (scriptStack.getArgumentsCount() == 1 + 1)
	{
		scriptStack.pushQuaternionBox(scriptStack.getQuaternion(1 + 1));
	}
	else
	{
		Quaternion quaternion = createQuaternion(scriptStack.getFloat(1 + 1)
			, scriptStack.getFloat(2 + 1)
			, scriptStack.getFloat(3 + 1)
			, scriptStack.getFloat(4 + 1)
			);
		scriptStack.pushQuaternionBox(quaternion);
	}

	return 1;
}

static int quaternionBox_store(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	Quaternion& quaternion = scriptStack.getQuaternionBox(1);

	if (scriptStack.getArgumentsCount() == 2)
	{
		quaternion = scriptStack.getQuaternion(2);
	}
	else
	{
		quaternion = createQuaternion(scriptStack.getFloat(2)
			, scriptStack.getFloat(3)
			, scriptStack.getFloat(4)
			, scriptStack.getFloat(5));
	}
	return 0;
}

static int quaternionBox_unbox(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	Quaternion& quaternion = scriptStack.getQuaternionBox(1);

	scriptStack.pushQuaternion(quaternion);
	return 1;
}

static int quaternionBox_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Quaternion& quaternion = scriptStack.getQuaternionBox(1);
	scriptStack.pushStringWithFormat("QuaternionBox (%p)", &quaternion);
	return 1;
}

// Color4
static int color4_constructor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Color4 color4;
	color4.x = scriptStack.getFloat(1 + 1);
	color4.y = scriptStack.getFloat(2 + 1);
	color4.z = scriptStack.getFloat(3 + 1);
	color4.w = scriptStack.getFloat(4 + 1);
	scriptStack.pushColor4(color4);
	return 1;
}

static int color4_createBlack(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_BLACK);
	return 1;
}

static int color4_createWhite(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_WHITE);
	return 1;
}

static int color4_createRed(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_RED);
	return 1;
}

static int color4_createGreen(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_GREEN);
	return 1;
}

static int color4_createBlue(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_BLUE);
	return 1;
}

static int color4_createYellow(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_YELLOW);
	return 1;
}

static int color4_createOrange(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(COLOR4_ORANGE);
	return 1;
}

static int lightUserData_add(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Vector3& a = scriptStack.getVector3(1);
	const Vector3& b = scriptStack.getVector3(2);
	scriptStack.pushVector3(a + b);
	return 1;
}

static int lightUserData_subtract(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Vector3& a = scriptStack.getVector3(1);
	const Vector3& b = scriptStack.getVector3(2);
	scriptStack.pushVector3(a - b);
	return 1;
}

static int lightUserData_multiply(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const int i = scriptStack.getIsNumber(1) ? 1 : 2;
	scriptStack.pushVector3(scriptStack.getFloat(i) * scriptStack.getVector3(3-i));
	return 1;
}

static int lightUserData_negate(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(-scriptStack.getVector3(1));
	return 1;
}

static int lightUserData_getIndex(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector3& vector3 = scriptStack.getVector3(1);
	const char* str = scriptStack.getString(2);

	switch (str[0])
	{
		case 'x': scriptStack.pushFloat(vector3.x); return 1;
		case 'y': scriptStack.pushFloat(vector3.y); return 1;
		case 'z': scriptStack.pushFloat(vector3.z); return 1;
		default: LUA_ASSERT(false, scriptStack, "Bad index: '%c'", str[0]); break;
	}

	return 0;
}

static int lightUserData_createNewIndex(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Vector3& vector3 = scriptStack.getVector3(1);
	const char* str = scriptStack.getString(2);
	const float value = scriptStack.getFloat(3);

	switch (str[0])
	{
		case 'x': vector3.x = value; break;
		case 'y': vector3.y = value; break;
		case 'z': vector3.z = value; break;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", str[0]); break;
	}

	return 0;
}

static int inputDevice_getName(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(inputDevice.getName());
	return 1;
}

static int inputDevice_getIsConnected(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(inputDevice.getIsConnected());
	return 1;
}

static int inputDevice_getButtonsCount(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushInteger(inputDevice.getButtonsCount());
	return 1;
}

static int inputDevice_getAxesCount(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushInteger(inputDevice.getAxesCount());
	return 1;
}

static int inputDevice_getIsPressed(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(inputDevice.getIsPressed(scriptStack.getInteger(1)));
	return 1;
}

static int inputDevice_getIsReleased(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(inputDevice.getIsReleased(scriptStack.getInteger(1)));
	return 1;
}

static int inputDevice_getIsAnyPressed(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(inputDevice.getIsAnyPressed());
	return 1;
}

static int inputDevice_getIsAnyReleased(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(inputDevice.getIsAnyReleased());
	return 1;
}

static int inputDevice_getAxis(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(inputDevice.getAxis(scriptStack.getInteger(1)));
	return 1;
}

static int inputDevice_getButtonName(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(inputDevice.getButtonName(scriptStack.getInteger(1)));
	return 1;
}

static int inputDevice_getAxisName(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(inputDevice.getAxisName(scriptStack.getInteger(1)));
	return 1;
}

static int inputDevice_getButtonId(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushInteger(inputDevice.getButtonId(scriptStack.getStringId32(1)));
	return 1;
}

static int inputDevice_getAxisId(lua_State* scriptState, InputDevice& inputDevice)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushInteger(inputDevice.getAxisId(scriptStack.getStringId32(1)));
	return 1;
}

#define KEYBOARD_FN(name) keyboard_##name
#define MOUSE_FN(name) mouse_##name
#define TOUCH_FN(name) touch_##name
#define JOYPAD_FN(index, name) joypad_##name##index

#define KEYBOARD(name) static int KEYBOARD_FN(name)(lua_State* scriptState)\
	{ return inputDevice_##name(scriptState, *getDevice()->getInputManager()->getKeyboard()); }
#define MOUSE(name) static int MOUSE_FN(name)(lua_State* scriptState)\
	{ return inputDevice_##name(scriptState, *getDevice()->getInputManager()->getMouse()); }
#define TOUCH(name) static int TOUCH_FN(name)(lua_State* scriptState)\
	{ return inputDevice_##name(scriptState, *getDevice()->getInputManager()->getTouch()); }
#define JOYPAD(index, name) static int JOYPAD_FN(index, name)(lua_State* scriptState)\
	{ return inputDevice_##name(scriptState, *getDevice()->getInputManager()->getJoypad(index)); }

KEYBOARD(getName)
KEYBOARD(getIsConnected)
KEYBOARD(getButtonsCount)
KEYBOARD(getAxesCount)
KEYBOARD(getIsPressed)
KEYBOARD(getIsReleased)
KEYBOARD(getIsAnyPressed)
KEYBOARD(getIsAnyReleased)
// KEYBOARD(getAxis)
KEYBOARD(getButtonName)
// KEYBOARD(getAxisName)
KEYBOARD(getButtonId)
// KEYBOARD(getAxisId)

MOUSE(getName)
MOUSE(getIsConnected)
MOUSE(getButtonsCount)
MOUSE(getAxesCount)
MOUSE(getIsPressed)
MOUSE(getIsReleased)
MOUSE(getIsAnyPressed)
MOUSE(getIsAnyReleased)
MOUSE(getAxis)
MOUSE(getButtonName)
MOUSE(getAxisName)
MOUSE(getButtonId)
MOUSE(getAxisId)

TOUCH(getName)
TOUCH(getIsConnected)
TOUCH(getButtonsCount)
TOUCH(getAxesCount)
TOUCH(getIsPressed)
TOUCH(getIsReleased)
TOUCH(getIsAnyPressed)
TOUCH(getIsAnyReleased)
TOUCH(getAxis)
TOUCH(getButtonName)
TOUCH(getAxisName)
TOUCH(getButtonId)
TOUCH(getAxisId)

JOYPAD(0, getName)
JOYPAD(0, getIsConnected)
JOYPAD(0, getButtonsCount)
JOYPAD(0, getAxesCount)
JOYPAD(0, getIsPressed)
JOYPAD(0, getIsReleased)
JOYPAD(0, getIsAnyPressed)
JOYPAD(0, getIsAnyReleased)
JOYPAD(0, getAxis)
JOYPAD(0, getButtonName)
JOYPAD(0, getAxisName)
JOYPAD(0, getButtonId)
JOYPAD(0, getAxisId)

JOYPAD(1, getName)
JOYPAD(1, getIsConnected)
JOYPAD(1, getButtonsCount)
JOYPAD(1, getAxesCount)
JOYPAD(1, getIsPressed)
JOYPAD(1, getIsReleased)
JOYPAD(1, getIsAnyPressed)
JOYPAD(1, getIsAnyReleased)
JOYPAD(1, getAxis)
JOYPAD(1, getButtonName)
JOYPAD(1, getAxisName)
JOYPAD(1, getButtonId)
JOYPAD(1, getAxisId)

JOYPAD(2, getName)
JOYPAD(2, getIsConnected)
JOYPAD(2, getButtonsCount)
JOYPAD(2, getAxesCount)
JOYPAD(2, getIsPressed)
JOYPAD(2, getIsReleased)
JOYPAD(2, getIsAnyPressed)
JOYPAD(2, getIsAnyReleased)
JOYPAD(2, getAxis)
JOYPAD(2, getButtonName)
JOYPAD(2, getAxisName)
JOYPAD(2, getButtonId)
JOYPAD(2, getAxisId)

JOYPAD(3, getName)
JOYPAD(3, getIsConnected)
JOYPAD(3, getButtonsCount)
JOYPAD(3, getAxesCount)
JOYPAD(3, getIsPressed)
JOYPAD(3, getIsReleased)
JOYPAD(3, getIsAnyPressed)
JOYPAD(3, getIsAnyReleased)
JOYPAD(3, getAxis)
JOYPAD(3, getButtonName)
JOYPAD(3, getAxisName)
JOYPAD(3, getButtonId)
JOYPAD(3, getAxisId)

static int world_spawnUnit(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const int argumentsCount = scriptStack.getArgumentsCount();

	const StringId64 name = scriptStack.getResourceId(2);
	const Vector3& position = argumentsCount > 2 ? scriptStack.getVector3(3) : VECTOR3_ZERO;
	const Quaternion& rotation = argumentsCount > 3 ? scriptStack.getQuaternion(4) : QUATERNION_IDENTITY;

	LUA_ASSERT(getDevice()->getResourceManager()->canGet(RESOURCE_TYPE_UNIT, name), scriptStack, "Unit not found");

	scriptStack.pushUnit(scriptStack.getWorld(1)->spawnUnit(name, position, rotation));
	return 1;
}

static int world_spawnEmptyUnit(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushUnit(scriptStack.getWorld(1)->spawnEmptyUnit());
	return 1;
}

static int world_destroyUnit(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->destroyUnit(scriptStack.getUnit(2));
	return 0;
}

static int world_getUnitListCount(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushInteger(scriptStack.getWorld(1)->getUnitListCount());
	return 1;
}

static int world_getUnitList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	TempAllocator1024 alloc;
	Array<UnitId> unitIdList(alloc);
	scriptStack.getWorld(1)->getAllUnits(unitIdList);

	const uint32_t num = ArrayFn::getCount(unitIdList);

	scriptStack.pushTable(num);
	for (uint32_t i = 0; i < num; ++i)
	{
		scriptStack.pushKeyBegin((int32_t) i + 1);
		scriptStack.pushUnit(unitIdList[i]);
		scriptStack.pushKeyEnd();
	}

	return 1;
}

static int world_getCamera(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushCamera(scriptStack.getWorld(1)->getCamera(scriptStack.getUnit(2)));
	return 1;
}

static int world_setCameraProjectionType(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	const char* name = scriptStack.getString(3);
	const ProjectionType::Enum projectionType = getProjectionTypeFromName(name);
	LUA_ASSERT(projectionType != ProjectionType::COUNT, scriptStack, "Unknown projection type: '%s'", name);

	scriptStack.getWorld(1)->setCameraProjectionType(scriptStack.getCamera(2), projectionType);
	return 0;
}

static int world_getCameraProjectionType(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	ProjectionType::Enum projectionType = scriptStack.getWorld(1)->getCameraProjectionType(scriptStack.getCamera(2));
	scriptStack.pushString(projectionInfoMap[projectionType].name);
	return 1;
}

static int world_getCameraFov(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getWorld(1)->getCameraFov(scriptStack.getCamera(2)));
	return 1;
}

static int world_setCameraFov(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setCameraFov(scriptStack.getCamera(2), scriptStack.getFloat(3));
	return 0;
}

static int world_getCameraAspect(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getWorld(1)->getCameraAspect(scriptStack.getCamera(2)));
	return 1;
}

static int world_setCameraAspect(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setCameraAspect(scriptStack.getCamera(2), scriptStack.getFloat(3));
	return 0;
}

static int world_getCameraNearClipDistance(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getWorld(1)->getCameraNearClipDistance(scriptStack.getCamera(2)));
	return 1;
}

static int world_setCameraNearClipDistance(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setCameraNearClipDistance(scriptStack.getCamera(2), scriptStack.getFloat(3));
	return 0;
}

static int world_getCameraFarClipDistance(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getWorld(1)->getCameraFarClipDistance(scriptStack.getCamera(2)));
	return 1;
}

static int world_setCameraFarClipDistance(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setCameraFarClipDistance(scriptStack.getCamera(2), scriptStack.getFloat(3));
	return 0;
}

static int world_setCameraOrthographicMetrics(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setCameraOrthographicMetrics(scriptStack.getCamera(2)
		, scriptStack.getFloat(3), scriptStack.getFloat(4)
		, scriptStack.getFloat(5), scriptStack.getFloat(6));
	return 0;
}

static int world_getCameraWorldFromScreen(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getWorld(1)->getCameraWorldFromScreen(scriptStack.getCamera(2), scriptStack.getVector3(3)));
	return 1;
}

static int world_getCameraScreenFromWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getWorld(1)->getCameraScreenFromWorld(scriptStack.getCamera(2), scriptStack.getVector3(3)));
	return 1;
}

static int world_updateAnimations(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->updateAnimations(scriptStack.getFloat(2));
	return 0;
}

static int world_updateScene(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->updateScene(scriptStack.getFloat(2));
	return 0;
}

static int world_update(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->update(scriptStack.getFloat(2));
	return 0;
}

static int world_playSound(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const int32_t argumentsCount = scriptStack.getArgumentsCount();
	World* world = scriptStack.getWorld(1);
	const StringId64 name = scriptStack.getResourceId(2);
	const bool loop = argumentsCount > 2 ? scriptStack.getBool(3) : false;
	const float volume = argumentsCount > 3 ? scriptStack.getFloat(4) : 1.0f;
	const Vector3& position = argumentsCount > 4 ? scriptStack.getVector3(5) : VECTOR3_ZERO;
	const float range = argumentsCount > 5 ? scriptStack.getFloat(6) : 1000.0f;

	LUA_ASSERT(getDevice()->getResourceManager()->canGet(RESOURCE_TYPE_SOUND, name), scriptStack, "Sound not found");

	scriptStack.pushSoundInstanceId(world->playSound(name, loop, volume, position, range));
	return 1;
}

static int world_stopSound(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->stopSound(scriptStack.getSoundInstanceId(2));
	return 0;
}

static int world_linkSound(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->linkSound(scriptStack.getSoundInstanceId(2)
		, scriptStack.getUnit(3)
		, scriptStack.getInteger(4)
		);
	return 0;
}

static int world_setListenerPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setListenerPose(scriptStack.getMatrix4x4(2));
	return 0;
}

static int world_setSoundPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setSoundPosition(scriptStack.getSoundInstanceId(2), scriptStack.getVector3(3));
	return 0;
}

static int world_setSoundRange(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setSoundRange(scriptStack.getSoundInstanceId(2), scriptStack.getFloat(3));
	return 0;
}

static int world_setSoundVolume(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->setSoundVolume(scriptStack.getSoundInstanceId(2), scriptStack.getFloat(3));
	return 0;
}

static int world_createDebugLine(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushDebugLine(scriptStack.getWorld(1)->createDebugLine(scriptStack.getBool(2)));
	return 1;
}

static int world_destroyDebugLine(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->destroyDebugLine(*scriptStack.getDebugLine(2));
	return 0;
}

static int world_createScreenGui(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushGui(scriptStack.getWorld(1)->createScreenGui(scriptStack.getFloat(2), scriptStack.getFloat(3)));
	return 1;
}

static int world_destroyGui(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getWorld(1)->destroyGui(*scriptStack.getGui(2));
	return 0;
}

static int world_loadLevel(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const int argumentsCount = scriptStack.getArgumentsCount();

	const StringId64 name = scriptStack.getResourceId(2);
	const Vector3& position = argumentsCount > 2 ? scriptStack.getVector3(3) : VECTOR3_ZERO;
	const Quaternion& rotation = argumentsCount > 3 ? scriptStack.getQuaternion(4) : QUATERNION_IDENTITY;
	LUA_ASSERT(getDevice()->getResourceManager()->canGet(RESOURCE_TYPE_LEVEL, name), scriptStack, "Level not found");
	scriptStack.pushLevel(scriptStack.getWorld(1)->loadLevel(name, position, rotation));
	return 1;
}

static int world_getSceneGraph(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushSceneGraph(scriptStack.getWorld(1)->getSceneGraph());
	return 1;
}

static int world_getRenderWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushRenderWorld(scriptStack.getWorld(1)->getRenderWorld());
	return 1;
}

static int world_getPhysicsWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushPhysicsWorld(scriptStack.getWorld(1)->getPhysicsWorld());
	return 1;
}

static int world_getSoundWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushSoundWorld(scriptStack.getWorld(1)->getSoundWorld());
	return 1;
}

static int world_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	World* world = scriptStack.getWorld(1);
	scriptStack.pushStringWithFormat("World (%p)", world);
	return 1;
}

static int sceneGraph_create(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	TransformInstance transformInstance = scriptStack.getSceneGraph(1)->create(scriptStack.getUnit(2)
		, scriptStack.getVector3(3)
		, scriptStack.getQuaternion(4)
		, scriptStack.getVector3(5)
		);
	scriptStack.pushTransform(transformInstance);
	return 1;
}

static int sceneGraph_destroy(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->destroy(scriptStack.getTransform(2));
	return 0;
}

static int sceneGraph_getTransformInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	SceneGraph* sceneGraph = scriptStack.getSceneGraph(1);
	TransformInstance transformInstance = sceneGraph->get(scriptStack.getUnit(2));
	if (sceneGraph->getIsValid(transformInstance))
	{
		scriptStack.pushTransform(transformInstance);
	}
	else
	{
		scriptStack.pushNil();
	}
	return 1;
}

static int sceneGraph_getLocalPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getSceneGraph(1)->getLocalPosition(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getLocalRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(scriptStack.getSceneGraph(1)->getLocalRotation(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getLocalScale(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getSceneGraph(1)->getLocalScale(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getLocalPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getSceneGraph(1)->getLocalPose(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getWorldPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getSceneGraph(1)->getWorldPosition(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getWorldRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(scriptStack.getSceneGraph(1)->getWorldRotation(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_getWorldPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getSceneGraph(1)->getWorldPose(scriptStack.getTransform(2)));
	return 1;
}

static int sceneGraph_setLocalPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->setLocalPosition(scriptStack.getTransform(2), scriptStack.getVector3(3));
	return 0;
}

static int sceneGraph_setLocalRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->setLocalRotation(scriptStack.getTransform(2), scriptStack.getQuaternion(3));
	return 0;
}

static int sceneGraph_setLocalScale(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->setLocalScale(scriptStack.getTransform(2), scriptStack.getVector3(3));
	return 0;
}

static int sceneGraph_setLocalPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->setLocalPose(scriptStack.getTransform(2), scriptStack.getMatrix4x4(3));
	return 0;
}

static int sceneGraph_link(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->link(scriptStack.getTransform(2), scriptStack.getTransform(3));
	return 0;
}

static int sceneGraph_unlink(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSceneGraph(1)->unlink(scriptStack.getTransform(2));
	return 0;
}

static int unitManager_create(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	if (scriptStack.getArgumentsCount() == 1)
	{
		scriptStack.pushUnit(getDevice()->getUnitManager()->create(*scriptStack.getWorld(1)));
	}
	else
	{
		scriptStack.pushUnit(getDevice()->getUnitManager()->create());
	}

	return 1;
}

static int unitManager_getIsAlive(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(getDevice()->getUnitManager()->getIsAlive(scriptStack.getUnit(1)));
	return 1;
}

static int renderWorld_createMesh(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	RenderWorld* renderWorld = scriptStack.getRenderWorld(1);
	UnitId unitId = scriptStack.getUnit(2);

	MeshRendererDesc meshRendererDesc;
	meshRendererDesc.meshResource = scriptStack.getResourceId(3);
	meshRendererDesc.geometryName = scriptStack.getStringId32(4);
	meshRendererDesc.materialResource = scriptStack.getResourceId(5);
	meshRendererDesc.visible = scriptStack.getBool(6);

	Matrix4x4 pose = scriptStack.getMatrix4x4(7);

	scriptStack.pushMeshInstance(renderWorld->createMesh(unitId, meshRendererDesc, pose));
	return 1;
}

static int renderWorld_destroyMesh(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->destroyMesh(scriptStack.getMeshInstance(2));
	return 0;
}

static int renderWorld_getMeshInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	RenderWorld* renderWorld = scriptStack.getRenderWorld(1);
	UnitId unitId = scriptStack.getUnit(2);

	TempAllocator512 ta;
	Array<MeshInstance> meshInstanceList(ta);
	renderWorld->getMeshInstanceList(unitId, meshInstanceList);

	scriptStack.pushTable(ArrayFn::getCount(meshInstanceList));
	for (uint32_t i = 0; i < ArrayFn::getCount(meshInstanceList); ++i)
	{
		scriptStack.pushKeyBegin(i+1);
		scriptStack.pushMeshInstance(meshInstanceList[i]);
		scriptStack.pushKeyEnd();
	}

	return 1;
}

static int renderWorld_getMeshObb(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	Obb obb = scriptStack.getRenderWorld(1)->getMeshObb(scriptStack.getMeshInstance(2));
	scriptStack.pushMatrix4x4(obb.transformMatrix);
	scriptStack.pushVector3(obb.halfExtents);
	return 2;
}

static int renderWorld_getMeshRaycast(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	RenderWorld* renderWorld = scriptStack.getRenderWorld(1);
	float t = renderWorld->getMeshRaycast(scriptStack.getMeshInstance(2)
		, scriptStack.getVector3(3)
		, scriptStack.getVector3(4)
		);
	scriptStack.pushFloat(t);
	return 1;
}

static int renderWorld_setMeshVisible(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setMeshVisible(scriptStack.getMeshInstance(2), scriptStack.getBool(3));
	return 0;
}

static int renderWorld_createSprite(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	RenderWorld* renderWorld = scriptStack.getRenderWorld(1);
	UnitId unitId = scriptStack.getUnit(2);

	SpriteRendererDesc spriteRendererDesc;
	spriteRendererDesc.spriteResourceName = scriptStack.getResourceId(3);
	spriteRendererDesc.materialResource = scriptStack.getResourceId(4);
	spriteRendererDesc.visible = scriptStack.getBool(5);

	Matrix4x4 pose = scriptStack.getMatrix4x4(6);

	scriptStack.pushSpriteInstance(renderWorld->createSprite(unitId, spriteRendererDesc, pose));
	return 1;
}

static int renderWorld_destroySprite(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->destroySprite(scriptStack.getSpriteInstance(2));
	return 0;
}

static int renderWorld_getSpriteInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	RenderWorld* renderWorld = scriptStack.getRenderWorld(1);
	UnitId unitId = scriptStack.getUnit(2);

	TempAllocator512 ta;
	Array<SpriteInstance> spriteInstanceList(ta);
	renderWorld->getSpriteInstanceList(unitId, spriteInstanceList);

	scriptStack.pushTable(ArrayFn::getCount(spriteInstanceList));
	for (uint32_t i = 0; i < ArrayFn::getCount(spriteInstanceList); ++i)
	{
		scriptStack.pushKeyBegin(i+1);
		scriptStack.pushSpriteInstance(spriteInstanceList[i]);
		scriptStack.pushKeyEnd();
	}

	return 1;
}

static int renderWorld_setSpriteFrame(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setSpriteFrame(scriptStack.getSpriteInstance(2), scriptStack.getInteger(3));
	return 0;
}

static int renderWorld_setSpriteVisible(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setSpriteVisible(scriptStack.getSpriteInstance(2), scriptStack.getBool(3));
	return 0;
}

static int renderWorld_createLight(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	const char* name = scriptStack.getString(3);
	const LightType::Enum lightType = getLightTypeFromName(name);
	LUA_ASSERT(lt != LightType::COUNT, scriptStack, "Unknown light type: '%s'", name);

	LightDesc lightDesc;
	lightDesc.type = lightType;
	lightDesc.range = scriptStack.getFloat(4);
	lightDesc.intensity = scriptStack.getFloat(5);
	lightDesc.spotAngle = scriptStack.getFloat(6);
	lightDesc.color = scriptStack.getVector3(7);

	Matrix4x4 pose = scriptStack.getMatrix4x4(8);

	scriptStack.pushLightInstance(scriptStack.getRenderWorld(1)->createLight(scriptStack.getUnit(2), lightDesc, pose));
	return 1;
}

static int renderWorld_destroyLight(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->destroyLight(scriptStack.getLightInstance(2));
	return 0;
}

static int renderWorld_getLightInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushLightInstance(scriptStack.getRenderWorld(1)->getLight(scriptStack.getUnit(2)));
	return 1;
}

static int renderWorld_getLightType(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	LightType::Enum type = scriptStack.getRenderWorld(1)->getLightType(scriptStack.getLightInstance(2));
	scriptStack.pushString(lightInfoMap[type].name);
	return 1;
}

static int renderWorld_getLightColor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushColor4(scriptStack.getRenderWorld(1)->getLightColor(scriptStack.getLightInstance(2)));
	return 1;
}

static int renderWorld_getLightRange(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getRenderWorld(1)->getLightRange(scriptStack.getLightInstance(2)));
	return 1;
}

static int renderWorld_getLightIntensity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getRenderWorld(1)->getLightIntensity(scriptStack.getLightInstance(2)));
	return 1;
}

static int renderWorld_getLightSpotAngle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getRenderWorld(1)->getLightSpotAngle(scriptStack.getLightInstance(2)));
	return 1;
}

static int renderWorld_setLightType(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	const char* name = scriptStack.getString(3);
	const LightType::Enum lightType = getLightTypeFromName(name);
	LUA_ASSERT(lightType != LightType::COUNT, stack, "Unknown light type: '%s'", name);

	scriptStack.getRenderWorld(1)->setLightType(scriptStack.getLightInstance(2), lightType);
	return 0;
}

static int renderWorld_setLightColor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setLightColor(scriptStack.getLightInstance(2), scriptStack.getColor4(3));
	return 0;
}

static int renderWorld_setLightRange(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setLightRange(scriptStack.getLightInstance(2), scriptStack.getFloat(3));
	return 0;
}

static int renderWorld_setLightIntensity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setLightIntensity(scriptStack.getLightInstance(2), scriptStack.getFloat(3));
	return 0;
}

static int renderWorld_setLightSpotAngle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->setLightSpotAngle(scriptStack.getLightInstance(2), scriptStack.getFloat(3));
	return 0;
}

static int renderWorld_enableDebugDrawing(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getRenderWorld(1)->enableDebugDrawing(scriptStack.getBool(2));
	return 0;
}

static int physicsWorld_getActorInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	ActorInstance actorInstance = scriptStack.getPhysicsWorld(1)->getActor(scriptStack.getUnit(2));

	if (actorInstance.i == UINT32_MAX)
	{
		scriptStack.pushNil();
	}
	else
	{
		scriptStack.pushActor(actorInstance);
	}

	return 1;
}

static int physicsWorld_getActorWorldPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getPhysicsWorld(1)->getActorWorldPosition(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getActorWorldRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushQuaternion(scriptStack.getPhysicsWorld(1)->getActorWorldRotation(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getActorWorldPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushMatrix4x4(scriptStack.getPhysicsWorld(1)->getActorWorldPose(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_teleportActorWorldPosition(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->teleportActorWorldPosition(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_teleportActorWorldRotation(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->teleportActorWorldRotation(scriptStack.getActor(2), scriptStack.getQuaternion(3));
	return 0;
}

static int physicsWorld_teleportActorWorldPose(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->teleportActorWorldPose(scriptStack.getActor(2), scriptStack.getMatrix4x4(3));
	return 0;
}

static int physicsWorld_getActorCenterOfMass(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getPhysicsWorld(1)->getActorCenterOfMass(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_enableActorGravity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->enableActorGravity(scriptStack.getActor(2));
	return 0;
}

static int physicsWorld_disableActorGravity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->disableActorGravity(scriptStack.getActor(2));
	return 0;
}

static int physicsWorld_enableActorCollision(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->enableActorCollision(scriptStack.getActor(2));
	return 0;
}

static int physicsWorld_disableActorCollision(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->disableActorCollision(scriptStack.getActor(2));
	return 0;
}

static int physicsWorld_setActorCollisionFilter(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorCollisionFilter(scriptStack.getActor(2), scriptStack.getStringId32(3));
	return 0;
}

static int physicsWorld_setActorKinematic(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorKinematic(scriptStack.getActor(2), scriptStack.getBool(3));
	return 0;
}

static int physicsWorld_moveActor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->moveActor(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_getIsStatic(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getPhysicsWorld(1)->getIsStatic(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getIsDynamic(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getPhysicsWorld(1)->getIsDynamic(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getIsKinematic(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getPhysicsWorld(1)->getIsKinematic(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getIsNonKinematic(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getPhysicsWorld(1)->getIsNonKinematic(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_getActorLinearDamping(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getPhysicsWorld(1)->getActorLinearDamping(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_setActorLinearDamping(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorLinearDamping(scriptStack.getActor(2), scriptStack.getFloat(3));
	return 0;
}

static int physicsWorld_getActorAngularDamping(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(scriptStack.getPhysicsWorld(1)->getActorAngularDamping(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_setActorAngularDamping(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorAngularDamping(scriptStack.getActor(2), scriptStack.getFloat(3));
	return 0;
}

static int physicsWorld_getActorLinearVelocity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getPhysicsWorld(1)->getActorLinearVelocity(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_setActorLinearVelocity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorLinearVelocity(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_getActorAngularVelocity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getPhysicsWorld(1)->getActorAngularVelocity(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_setActorAngularVelocity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setActorAngularVelocity(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_addActorImpulse(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->addActorImpulse(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_addActorImpulseAt(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->addActorImpulseAt(scriptStack.getActor(2), scriptStack.getVector3(3), scriptStack.getVector3(4));
	return 0;
}

static int physicsWorld_addActorTorqueImpulse(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->addActorTorqueImpulse(scriptStack.getActor(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_pushActor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->pushActor(scriptStack.getActor(2), scriptStack.getVector3(3), scriptStack.getFloat(4));
	return 0;
}

static int physicsWorld_pushActorAt(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->pushActorAt(scriptStack.getActor(2), scriptStack.getVector3(3), scriptStack.getFloat(4), scriptStack.getVector3(5));
	return 0;
}

static int physicsWorld_getIsSleeping(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getPhysicsWorld(1)->getIsSleeping(scriptStack.getActor(2)));
	return 1;
}

static int physicsWorld_wakeUp(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->wakeUp(scriptStack.getActor(2));
	return 0;
}

static int physicsWorld_getControllerInstanceList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushController(scriptStack.getPhysicsWorld(1)->getController(scriptStack.getUnit(2)));
	return 1;
}

static int physicsWorld_moveController(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->moveController(scriptStack.getController(2), scriptStack.getVector3(3));
	return 0;
}

static int physicsWorld_createJoint(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	JointDesc jointDesc;
	jointDesc.type = JointType::SPRING;
	jointDesc.anchor0 = createVector3(0, -2, 0);
	jointDesc.anchor1 = createVector3(0, 2, 0);
	jointDesc.breakForce = 999999.0f;
	jointDesc.hinge.axis = createVector3(1, 0, 0);
	jointDesc.hinge.lowerLimit = -3.14f / 4.0f;
	jointDesc.hinge.upperLimit = 3.14f / 4.0f;
	jointDesc.hinge.bounciness = 12.0f;
	scriptStack.getPhysicsWorld(1)->createJoint(scriptStack.getActor(2), scriptStack.getActor(3), jointDesc);
	return 0;
}

static int physicsWorld_getGravity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector3(scriptStack.getPhysicsWorld(1)->getGravity());
	return 1;
}

static int physicsWorld_setGravity(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->setGravity(scriptStack.getVector3(2));
	return 0;
}

static int physicsWorld_raycast(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	PhysicsWorld* physicsWorld = scriptStack.getPhysicsWorld(1);

	const char* name = scriptStack.getString(5);
	const RaycastMode::Enum raycastMode = getRaycastModeFromName(name);
	LUA_ASSERT(raycastMode != RaycastMode::COUNT, scriptStack, "Unknown raycast mode: '%s'", name);

	TempAllocator1024 ta;
	Array<RaycastHit> raycastHitList(ta);

	physicsWorld->raycast(scriptStack.getVector3(2)
		, scriptStack.getVector3(3)
		, scriptStack.getFloat(4)
		, raycastMode
		, raycastHitList
		);

	scriptStack.pushTable();
	for (uint32_t i = 0; i < ArrayFn::getCount(raycastHitList); ++i)
	{
		scriptStack.pushKeyBegin(i+1);
		scriptStack.pushActor(raycastHitList[i].actor);
		scriptStack.pushKeyEnd();
	}

	return 1;
}

static int physicsWorld_enableDebugDrawing(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getPhysicsWorld(1)->enableDebugDrawing(scriptStack.getBool(2));
	return 0;
}

static int physicsWorld_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	PhysicsWorld* physicsWorld = scriptStack.getPhysicsWorld(1);
	scriptStack.pushStringWithFormat("PhysicsWorld (%p)", physicsWorld);
	return 1;
}

static int soundWorld_stopAll(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSoundWorld(1)->stopAll();
	return 0;
}

static int soundWorld_pauseAll(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSoundWorld(1)->pauseAll();
	return 0;
}

static int soundWorld_resumeAll(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getSoundWorld(1)->resumeAll();
	return 0;
}

static int soundWorld_getIsPlaying(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getSoundWorld(1)->getIsPlaying(scriptStack.getSoundInstanceId(2)));
	return 1;
}

static int soundWorld_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	SoundWorld* soundWorld = scriptStack.getSoundWorld(1);
	scriptStack.pushStringWithFormat("SoundWorld (%p)", soundWorld);
	return 1;
}

static int device_getArgumentList(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const int argumentListCount = getDevice()->getCommandLineArgumentListCount();
	const char** argumentList = getDevice()->getCommandLineArgumentList();
	scriptStack.pushTable(argumentListCount);
	for (int i = 0; i < argumentListCount; ++i)
	{
		scriptStack.pushKeyBegin(i + 1);
		scriptStack.pushString(argumentList[i]);
		scriptStack.pushKeyEnd();
	}
	return 1;
}

static int device_getPlatform(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(getDevice()->getPlatform());
	return 1;
}

static int device_getArchitecture(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(getDevice()->getArchitecture());
	return 1;
}

static int device_getVersion(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(getDevice()->getVersion());
	return 1;
}

static int device_getLastDeltaTime(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushFloat(getDevice()->getLastDeltaTime());
	return 1;
}

static int device_quit(lua_State* /*scriptState*/)
{
	getDevice()->quit();
	return 0;
}

static int device_getResolution(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	uint16_t w = 0;
	uint16_t h = 0;
	getDevice()->getResolution(w, h);
	scriptStack.pushInteger(w);
	scriptStack.pushInteger(h);
	return 2;
}

static int device_createWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushWorld(getDevice()->createWorld());
	return 1;
}

static int device_destroyWorld(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->destroyWorld(*scriptStack.getWorld(1));
	return 0;
}

static int device_render(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->render(*scriptStack.getWorld(1), scriptStack.getCamera(2));
	return 0;
}

static int device_createResourcePackage(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushResourcePackage(getDevice()->createResourcePackage(scriptStack.getResourceId(1)));
	return 1;
}

static int device_destroyResourcePackage(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->destroyResourcePackage(*scriptStack.getResourcePackage(1));
	return 0;
}

static void dumpScriptTable(lua_State* scriptState, int i, StringStream& jsonStringStream)
{
	ScriptStack scriptStack(scriptState);

	bool comma = false;
	int arrayIndex = 1;

	jsonStringStream << "{";

	scriptStack.pushNil();
	while (scriptStack.getNext(i) != 0)
	{
		if (comma == true)
		{
			jsonStringStream << ",";
		}
		comma = true;

		jsonStringStream << "\"";
		if (scriptStack.getIsString(-2) && !scriptStack.getIsNumber(-2))
		{
			jsonStringStream << scriptStack.getString(-2);
		}
		else
		{
			jsonStringStream << arrayIndex++;
		}
		jsonStringStream << "\":";

		if (scriptStack.getIsNil(i + 2))
		{
			jsonStringStream << "null";
		}
		else if (scriptStack.getIsBool(i + 2))
		{
			const bool b = scriptStack.getBool(i + 2);
			jsonStringStream << (b ? "true" : "false");
		}
		else if (scriptStack.getIsNumber(i + 2))
		{
			jsonStringStream << scriptStack.getFloat(i + 2);
		}
		else if (scriptStack.getIsString(i + 2))
		{
			const char* str = scriptStack.getString(i + 2);
			jsonStringStream << "\"";
			for (; *str; ++str)
			{
				if (*str == '"' || *str == '\\')
				{
					jsonStringStream << '\\';
				}
				jsonStringStream << *str;
			}
			jsonStringStream << "\"";
		}
		else if (scriptStack.getIsVector3(i + 2))
		{
			const Vector3 v = scriptStack.getVector3(i + 2);
			jsonStringStream << "["
				 << v.x << ","
				 << v.y << ","
				 << v.z
				 << "]"
				 ;
		}
		else if (scriptStack.getIsQuaternion(i + 2))
		{
			const Quaternion q = scriptStack.getQuaternion(i + 2);
			jsonStringStream << "["
				 << q.x << ","
				 << q.y << ","
				 << q.z << ","
				 << q.w
				 << "]"
				 ;
		}
		else if (scriptStack.getIsMatrix4x4(i + 2))
		{
			const Matrix4x4 m = scriptStack.getMatrix4x4(i + 2);
			jsonStringStream << "["
				 << m.x.x << ","
				 << m.x.y << ","
				 << m.x.z << ","
				 << m.x.w << ","

				 << m.y.x << ","
				 << m.y.y << ","
				 << m.y.z << ","
				 << m.y.w << ","

				 << m.z.x << ","
				 << m.z.y << ","
				 << m.z.z << ","
				 << m.z.w << ","

				 << m.t.x << ","
				 << m.t.y << ","
				 << m.t.z << ","
				 << m.t.w
				 << "]"
				 ;
		}
		else if (scriptStack.getIsTable(i + 2))
		{
			dumpScriptTable(scriptState, i + 2, jsonStringStream);
		}
		else
		{
			LUA_ASSERT(false, scriptStack, "Unsupported key value");
		}

		scriptStack.pop(1);
	}

	jsonStringStream << "}";
}

static int device_sendWithConsoleServer(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	LUA_ASSERT(scriptStack.getIsTable(1), scriptStack, "Table expected");

	TempAllocator1024 ta;
	StringStream jsonStringStream(ta);
	dumpScriptTable(scriptState, 1, jsonStringStream);

	getDevice()->getConsoleServer()->send(StringStreamFn::getCStr(jsonStringStream));
	return 0;
}

static int device_canGet(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const StringId64 type(scriptStack.getString(1));
	scriptStack.pushBool(getDevice()->getResourceManager()->canGet(type, scriptStack.getResourceId(2)));
	return 1;
}

static int device_enableResourceAutoload(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getResourceManager()->enableResourceAutoload(scriptStack.getBool(1));
	return 0;
}

static int device_getTemporaryObjectsCount(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	uint32_t vectorsCount;
	uint32_t quaternionsCount;
	uint32_t matricesCount;
	getDevice()->getScriptEnvironment()->getTemporaryObjectsCount(vectorsCount, quaternionsCount, matricesCount);
	scriptStack.pushInteger(vectorsCount);
	scriptStack.pushInteger(quaternionsCount);
	scriptStack.pushInteger(matricesCount);
	return 3;
}

static int device_setTemporaryObjectsCount(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	uint32_t vectorsCount = scriptStack.getInteger(1);
	uint32_t quaternionsCount = scriptStack.getInteger(2);
	uint32_t matricesCount = scriptStack.getInteger(3);
	getDevice()->getScriptEnvironment()->setTemporaryObjectsCount(vectorsCount, quaternionsCount, matricesCount);
	return 0;
}

static int device_getGuid(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	TempAllocator64 ta;
	DynamicString str(ta);
	Guid guid = GuidFn::createGuid();
	GuidFn::toString(guid, str);
	scriptStack.pushString(str.getCStr());
	return 1;
}

static int profiler_enterProfileScope(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	ProfilerFn::enterProfileScope(scriptStack.getString(1));
	return 0;
}

static int profiler_leaveProfileScope(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	ProfilerFn::leaveProfileScope();
	return 0;
}

static int profiler_record(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);

	const char* name = scriptStack.getString(1);

	if (scriptStack.getIsNumber(2))
	{
		ProfilerFn::recordFloat(name, scriptStack.getFloat(2));
	}
	else
	{
		ProfilerFn::recordVector3(name, scriptStack.getVector3(2));
	}

	return 0;
}

static int debugLine_addLine(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getDebugLine(1)->addLine(scriptStack.getVector3(2)
		, scriptStack.getVector3(3)
		, scriptStack.getColor4(4)
		);
	return 0;
}

static int debugLine_addAxes(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const float length = scriptStack.getArgumentsCount() == 3
		? scriptStack.getFloat(3)
		: 1.0f
		;
	scriptStack.getDebugLine(1)->addAxes(scriptStack.getMatrix4x4(2), length);
	return 0;
}

static int debugLine_addCircle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const uint32_t segments = scriptStack.getArgumentsCount() >= 6
		? scriptStack.getInteger(6)
		: DebugLine::NUM_SEGMENTS
		;
	scriptStack.getDebugLine(1)->addCircle(scriptStack.getVector3(2)
		, scriptStack.getFloat(3)
		, scriptStack.getVector3(4)
		, scriptStack.getColor4(5)
		, segments
		);
	return 0;
}

static int debugLine_addCone(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const uint32_t segments = scriptStack.getArgumentsCount() >= 6
		? scriptStack.getInteger(6)
		: DebugLine::NUM_SEGMENTS
		;
	scriptStack.getDebugLine(1)->addCone(scriptStack.getVector3(2)
		, scriptStack.getVector3(3)
		, scriptStack.getFloat(4)
		, scriptStack.getColor4(5)
		, segments
		);
	return 0;
}

static int debugLine_addSphere(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const uint32_t segments = scriptStack.getArgumentsCount() >= 5
		? scriptStack.getInteger(5)
		: DebugLine::NUM_SEGMENTS
		;
	scriptStack.getDebugLine(1)->addSphere(scriptStack.getVector3(2)
		, scriptStack.getFloat(3)
		, scriptStack.getColor4(4)
		, segments
		);
	return 0;
}

static int debugLine_addObb(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getDebugLine(1)->addObb(scriptStack.getMatrix4x4(2)
		, scriptStack.getVector3(3)
		, scriptStack.getColor4(4)
		);
	return 0;
}

static int debugLine_addUnit(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getDebugLine(1)->addUnit(*getDevice()->getResourceManager()
		, scriptStack.getMatrix4x4(2)
		, scriptStack.getResourceId(3)
		, scriptStack.getColor4(4)
		);
	return 0;
}

static int debugLine_reset(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getDebugLine(1)->reset();
	return 0;
}

static int debugLine_submit(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getDebugLine(1)->submit();
	return 0;
}

static int debugLine_toString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushStringWithFormat("DebugLine (%p)", scriptStack.getDebugLine(1));
	return 1;
}

static int resourcePackage_load(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getResourcePackage(1)->load();
	return 0;
}

static int resourcePackage_unload(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getResourcePackage(1)->unload();
	return 0;
}

static int resourcePackage_flush(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getResourcePackage(1)->flush();
	return 0;
}

static int resourcePackage_hasLoaded(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushBool(scriptStack.getResourcePackage(1)->hasLoaded());
	return 1;
}

static int resourcePackage_getString(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	ResourcePackage* resourcePackage = scriptStack.getResourcePackage(1);
	scriptStack.pushStringWithFormat("ResourcePackage (%p)", resourcePackage);
	return 1;
}

static int material_setFloat(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getMaterial(1)->setFloat(scriptStack.getStringId32(2), scriptStack.getFloat(3));
	return 0;
}

static int material_setVector2(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getMaterial(1)->setVector2(scriptStack.getStringId32(2), scriptStack.getVector2(3));
	return 0;
}

static int material_setVector3(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getMaterial(1)->setVector3(scriptStack.getStringId32(2), scriptStack.getVector3(3));
	return 0;
}

static int gui_getResolution(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	const Vector2 resolution = scriptStack.getGui(1)->getResolution();
	scriptStack.pushInteger((int32_t)resolution.x);
	scriptStack.pushInteger((int32_t)resolution.y);
	return 2;
}

static int gui_move(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getGui(1)->move(scriptStack.getVector2(2));
	return 0;
}

static int gui_getGuiFromScreen(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushVector2(scriptStack.getGui(1)->getGuiFromScreen(scriptStack.getVector2(2)));
	return 1;
}

static int gui_drawRectangle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getGui(1)->drawRectangle(scriptStack.getVector2(2)
		, scriptStack.getVector2(3)
		, scriptStack.getResourceId(4)
		, scriptStack.getColor4(5)
		);
	return 0;
}

static int gui_drawImage(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getGui(1)->drawImage(scriptStack.getVector2(2)
		, scriptStack.getVector2(3)
		, scriptStack.getResourceId(4)
		, scriptStack.getColor4(5)
		);
	return 0;
}

static int gui_drawImageUv(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getGui(1)->drawImageUv(scriptStack.getVector2(2)
		, scriptStack.getVector2(3)
		, scriptStack.getVector2(4)
		, scriptStack.getVector2(5)
		, scriptStack.getResourceId(6)
		, scriptStack.getColor4(7)
		);
	return 0;
}

static int gui_drawText(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.getGui(1)->drawText(scriptStack.getVector2(2)
		, scriptStack.getInteger(3)
		, scriptStack.getString(4)
		, scriptStack.getResourceId(5)
		, scriptStack.getResourceId(6)
		, scriptStack.getColor4(7)
		);
	return 0;
}

static int display_getModes(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	TempAllocator1024 ta;
	Array<DisplayMode> displayModeList(ta);
	getDevice()->getMainDisplay().getModes(displayModeList);
	scriptStack.pushTable(ArrayFn::getCount(displayModeList));
	for (uint32_t i = 0; i < ArrayFn::getCount(displayModeList); ++i)
	{
		scriptStack.pushKeyBegin(i+1);
		scriptStack.pushTable(3);
		{
			scriptStack.pushKeyBegin("id");
			scriptStack.pushInteger(displayModeList[i].id);
			scriptStack.pushKeyEnd();

			scriptStack.pushKeyBegin("width");
			scriptStack.pushInteger(displayModeList[i].width);
			scriptStack.pushKeyEnd();

			scriptStack.pushKeyBegin("height");
			scriptStack.pushInteger(displayModeList[i].height);
			scriptStack.pushKeyEnd();
		}
		scriptStack.pushKeyEnd();
	}
	return 1;
}

static int display_setMode(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainDisplay().setMode(scriptStack.getInteger(1));
	return 0;
}

static int window_show(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->show();
	return 0;
}

static int window_hide(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->hide();
	return 0;
}

static int window_resize(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->resize(scriptStack.getInteger(1), scriptStack.getInteger(2));
	return 0;
}

static int window_move(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->move(scriptStack.getInteger(1), scriptStack.getInteger(2));
	return 0;
}

static int window_minimize(lua_State* /*scriptState*/)
{
	getDevice()->getMainWindow()->minimize();
	return 0;
}

static int window_restore(lua_State* /*scriptState*/)
{
	getDevice()->getMainWindow()->restore();
	return 0;
}

static int window_getTitle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	scriptStack.pushString(getDevice()->getMainWindow()->getTitle());
	return 1;
}

static int window_setTitle(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->setTitle(scriptStack.getString(1));
	return 0;
}

static int window_showCursor(lua_State* scriptState)
{
	ScriptStack scriptStack(scriptState);
	getDevice()->getMainWindow()->setShowCursor(scriptStack.getBool(1));
	return 0;
}

void loadApi(ScriptEnvironment& scriptEnvironment)
{
	scriptEnvironment.addModuleFunction("Math", "getRayPlaneIntersection", math_getRayPlaneIntersection);
	scriptEnvironment.addModuleFunction("Math", "getRayDiscIntersection", math_getRayDiscIntersection);
	scriptEnvironment.addModuleFunction("Math", "getRaySphereIntersection", math_getRaySphereIntersection);
	scriptEnvironment.addModuleFunction("Math", "getRayObbIntersection", math_getRayObbIntersection);
	scriptEnvironment.addModuleFunction("Math", "getRayTriangleIntersection", math_getRayTriangleIntersection);

	scriptEnvironment.addModuleFunction("Vector3", "getX", vector3_getX);
	scriptEnvironment.addModuleFunction("Vector3", "getY", vector3_getY);
	scriptEnvironment.addModuleFunction("Vector3", "getZ", vector3_getZ);
	scriptEnvironment.addModuleFunction("Vector3", "setX", vector3_setX);
	scriptEnvironment.addModuleFunction("Vector3", "setY", vector3_setY);
	scriptEnvironment.addModuleFunction("Vector3", "setZ", vector3_setZ);
	scriptEnvironment.addModuleFunction("Vector3", "getElements", vector3_getElements);
	scriptEnvironment.addModuleFunction("Vector3", "add", vector3_add);
	scriptEnvironment.addModuleFunction("Vector3", "subtract", vector3_subtract);
	scriptEnvironment.addModuleFunction("Vector3", "multiply", vector3_multiply);
	scriptEnvironment.addModuleFunction("Vector3", "dot", vector3_dot);
	scriptEnvironment.addModuleFunction("Vector3", "cross", vector3_cross);
	scriptEnvironment.addModuleFunction("Vector3", "getAreEqual", vector3_getAreEqual);
	scriptEnvironment.addModuleFunction("Vector3", "getLength", vector3_getLength);
	scriptEnvironment.addModuleFunction("Vector3", "getLengthSquared", vector3_getLengthSquared);
	scriptEnvironment.addModuleFunction("Vector3", "setLength", vector3_setLength);
	scriptEnvironment.addModuleFunction("Vector3", "normalize", vector3_normalize);
	scriptEnvironment.addModuleFunction("Vector3", "getDistance", vector3_getDistance);
	scriptEnvironment.addModuleFunction("Vector3", "getDistanceSquared", vector3_getDistanceSquared);
	scriptEnvironment.addModuleFunction("Vector3", "getAngle", vector3_getAngle);
	scriptEnvironment.addModuleFunction("Vector3", "max", vector3_max);
	scriptEnvironment.addModuleFunction("Vector3", "min", vector3_min);
	scriptEnvironment.addModuleFunction("Vector3", "getLinearInterpolation", vector3_getLinearInterpolation);
	scriptEnvironment.addModuleFunction("Vector3", "createForward", vector3_createForward);
	scriptEnvironment.addModuleFunction("Vector3", "createBackward", vector3_createBackward);
	scriptEnvironment.addModuleFunction("Vector3", "createLeft", vector3_createLeft);
	scriptEnvironment.addModuleFunction("Vector3", "createRight", vector3_createRight);
	scriptEnvironment.addModuleFunction("Vector3", "createUp", vector3_createUp);
	scriptEnvironment.addModuleFunction("Vector3", "createDown", vector3_createDown);
	scriptEnvironment.addModuleFunction("Vector3", "createZero", vector3_createZero);
	scriptEnvironment.addModuleFunction("Vector3", "getString", vector3_getString);

	scriptEnvironment.setModuleConstructor("Vector3", vector3_constructor);

	scriptEnvironment.setModuleConstructor("Vector2", vector2_constructor);

	scriptEnvironment.addModuleFunction("Vector3Box", "store", vector3box_store);
	scriptEnvironment.addModuleFunction("Vector3Box", "unbox", vector3box_unbox);
	scriptEnvironment.addModuleFunction("Vector3Box", "__getIndex", "Vector3Box");
	scriptEnvironment.addModuleFunction("Vector3Box", "__getString", vector3box_getString);

	scriptEnvironment.setModuleConstructor("Vector3Box", vector3box_constructor);

	scriptEnvironment.addModuleFunction("Matrix4x4", "createFromQuaternion", matrix4x4_createFromQuaternion);
	scriptEnvironment.addModuleFunction("Matrix4x4", "createFromTranslation", matrix4x4_createFromTranslation);
	scriptEnvironment.addModuleFunction("Matrix4x4", "createFromQuaternionTranslation", matrix4x4_createFromQuaternionTranslation);
	scriptEnvironment.addModuleFunction("Matrix4x4", "createFromAxes", matrix4x4_createFromAxes);
	scriptEnvironment.addModuleFunction("Matrix4x4", "copy", matrix4x4_copy);
	scriptEnvironment.addModuleFunction("Matrix4x4", "add", matrix4x4_add);
	scriptEnvironment.addModuleFunction("Matrix4x4", "subtract", matrix4x4_subtract);
	scriptEnvironment.addModuleFunction("Matrix4x4", "multiply", matrix4x4_multiply);
	scriptEnvironment.addModuleFunction("Matrix4x4", "transpose", matrix4x4_transpose);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getDeterminant", matrix4x4_getDeterminant);
	scriptEnvironment.addModuleFunction("Matrix4x4", "invert", matrix4x4_invert);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getAxisX", matrix4x4_getAxisX);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getAxisY", matrix4x4_getAxisY);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getAxisZ", matrix4x4_getAxisZ);
	scriptEnvironment.addModuleFunction("Matrix4x4", "setAxisX", matrix4x4_setAxisX);
	scriptEnvironment.addModuleFunction("Matrix4x4", "setAxisY", matrix4x4_setAxisY);
	scriptEnvironment.addModuleFunction("Matrix4x4", "setAxisZ", matrix4x4_setAxisZ);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getTranslation", matrix4x4_getTranslation);
	scriptEnvironment.addModuleFunction("Matrix4x4", "setTranslation", matrix4x4_setTranslation);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getRotationAsQuaternion", matrix4x4_getRotationAsQuaternion);
	scriptEnvironment.addModuleFunction("Matrix4x4", "setRotation", matrix4x4_setRotation);
	scriptEnvironment.addModuleFunction("Matrix4x4", "createIdentity", matrix4x4_createIdentity);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getTransform", matrix4x4_getTransform);
	scriptEnvironment.addModuleFunction("Matrix4x4", "getString", matrix4x4_getString);

	scriptEnvironment.setModuleConstructor("Matrix4x4", matrix4x4_constructor);

	scriptEnvironment.addModuleFunction("Matrix4x4Box", "store", matrix4x4Box_store);
	scriptEnvironment.addModuleFunction("Matrix4x4Box", "unbox", matrix4x4Box_unbox);
	scriptEnvironment.addModuleFunction("Matrix4x4Box", "__getIndex", "Matrix4x4Box");
	scriptEnvironment.addModuleFunction("Matrix4x4Box", "__getString", matrix4x4Box_getString);

	scriptEnvironment.setModuleConstructor("Matrix4x4Box", matrix4x4Box_constructor);

	scriptEnvironment.addModuleFunction("Quaternion", "createFromAxisAngle", quaternion_createFromAxisAngle);
	scriptEnvironment.addModuleFunction("Quaternion", "createFromElements", quaternion_createFromElements);
	scriptEnvironment.addModuleFunction("Quaternion", "createIdentity", quaternion_createIdentity);
	scriptEnvironment.addModuleFunction("Quaternion", "negate", quaternion_negate);
	scriptEnvironment.addModuleFunction("Quaternion", "dot", quaternion_dot);
	scriptEnvironment.addModuleFunction("Quaternion", "getLength", quaternion_getLength);
	scriptEnvironment.addModuleFunction("Quaternion", "normalize", quaternion_normalize);
	scriptEnvironment.addModuleFunction("Quaternion", "getConjugate", quaternion_getConjugate);
	scriptEnvironment.addModuleFunction("Quaternion", "getInverse", quaternion_getInverse);
	scriptEnvironment.addModuleFunction("Quaternion", "multiply", quaternion_multiply);
	scriptEnvironment.addModuleFunction("Quaternion", "multiplyByScalar", quaternion_multiplyByScalar);
	scriptEnvironment.addModuleFunction("Quaternion", "getPower", quaternion_getPower);
	scriptEnvironment.addModuleFunction("Quaternion", "getElementList", quaternion_getElementList);
	scriptEnvironment.addModuleFunction("Quaternion", "createLook", quaternion_createLook);
	scriptEnvironment.addModuleFunction("Quaternion", "createRight", quaternion_createRight);
	scriptEnvironment.addModuleFunction("Quaternion", "createUp", quaternion_createUp);
	scriptEnvironment.addModuleFunction("Quaternion", "createForward", quaternion_createForward);
	scriptEnvironment.addModuleFunction("Quaternion", "getLinearInterpolation", quaternion_getLinearInterpolation);
	scriptEnvironment.addModuleFunction("Quaternion", "toString", quaternion_getString);

	scriptEnvironment.setModuleConstructor("Quaternion", quaternion_constructor);

	scriptEnvironment.addModuleFunction("QuaternionBox", "store", quaternionBox_store);
	scriptEnvironment.addModuleFunction("QuaternionBox", "unbox", quaternionBox_unbox);
	scriptEnvironment.addModuleFunction("QuaternionBox", "__getIndex", "QuaternionBox");
	scriptEnvironment.addModuleFunction("QuaternionBox", "__getString", quaternionBox_getString);

	scriptEnvironment.setModuleConstructor("QuaternionBox", quaternionBox_constructor);

	scriptEnvironment.addModuleFunction("Color4", "createBlack", color4_createBlack);
	scriptEnvironment.addModuleFunction("Color4", "createWhite", color4_createWhite);
	scriptEnvironment.addModuleFunction("Color4", "createRed", color4_createRed);
	scriptEnvironment.addModuleFunction("Color4", "createGreen", color4_createGreen);
	scriptEnvironment.addModuleFunction("Color4", "createBlue", color4_createBlue);
	scriptEnvironment.addModuleFunction("Color4", "createYellow", color4_createYellow);
	scriptEnvironment.addModuleFunction("Color4", "createOrange", color4_createOrange);
	scriptEnvironment.addModuleFunction("Color4", "toString", quaternion_getString);

	scriptEnvironment.setModuleConstructor("Color4", color4_constructor);

	scriptEnvironment.addModuleFunction("LightUserData_mt", "__add", lightUserData_add);
	scriptEnvironment.addModuleFunction("LightUserData_mt", "__subtract", lightUserData_subtract);
	scriptEnvironment.addModuleFunction("LightUserData_mt", "__multiply", lightUserData_multiply);
	scriptEnvironment.addModuleFunction("LightUserData_mt", "__negate", lightUserData_negate);
	scriptEnvironment.addModuleFunction("LightUserData_mt", "__getIndex", lightUserData_getIndex);
	scriptEnvironment.addModuleFunction("LightUserData_mt", "__createNewIndex", lightUserData_createNewIndex);

	scriptEnvironment.addModuleFunction("Keyboard", "getName", KEYBOARD_FN(getName));
	scriptEnvironment.addModuleFunction("Keyboard", "getIsConnected",    KEYBOARD_FN(getIsConnected));
	scriptEnvironment.addModuleFunction("Keyboard", "getButtonsCount",  KEYBOARD_FN(getButtonsCount));
	scriptEnvironment.addModuleFunction("Keyboard", "getAxesCount",     KEYBOARD_FN(getAxesCount));
	scriptEnvironment.addModuleFunction("Keyboard", "getIsPressed",      KEYBOARD_FN(getIsPressed));
	scriptEnvironment.addModuleFunction("Keyboard", "getIsReleased",     KEYBOARD_FN(getIsReleased));
	scriptEnvironment.addModuleFunction("Keyboard", "getIsAnyPressed",  KEYBOARD_FN(getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Keyboard", "getIsAnyReleased", KEYBOARD_FN(getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Keyboard", "getButtonName",  KEYBOARD_FN(getButtonName));
	scriptEnvironment.addModuleFunction("Keyboard", "getButtonId",    KEYBOARD_FN(getButtonId));

	scriptEnvironment.addModuleFunction("Mouse", "getName", MOUSE_FN(getName));
	scriptEnvironment.addModuleFunction("Mouse", "getIsConnected",    MOUSE_FN(getIsConnected));
	scriptEnvironment.addModuleFunction("Mouse", "getButtonsCount",  MOUSE_FN(getButtonsCount));
	scriptEnvironment.addModuleFunction("Mouse", "getAxesCount",     MOUSE_FN(getAxesCount));
	scriptEnvironment.addModuleFunction("Mouse", "getIsPressed",      MOUSE_FN(getIsPressed));
	scriptEnvironment.addModuleFunction("Mouse", "getIsReleased",     MOUSE_FN(getIsReleased));
	scriptEnvironment.addModuleFunction("Mouse", "getIsAnyPressed",  MOUSE_FN(getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Mouse", "getIsAnyReleased", MOUSE_FN(getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Mouse", "getAxis",         MOUSE_FN(getAxis));
	scriptEnvironment.addModuleFunction("Mouse", "getButtonName",  MOUSE_FN(getButtonName));
	scriptEnvironment.addModuleFunction("Mouse", "getAxisName",    MOUSE_FN(getAxisName));
	scriptEnvironment.addModuleFunction("Mouse", "getButtonId",    MOUSE_FN(getButtonId));
	scriptEnvironment.addModuleFunction("Mouse", "getAxisId",      MOUSE_FN(getAxisId));

	scriptEnvironment.addModuleFunction("Touch", "getName", TOUCH_FN(getName));
	scriptEnvironment.addModuleFunction("Touch", "getIsConnected",    TOUCH_FN(getIsConnected));
	scriptEnvironment.addModuleFunction("Touch", "getButtonsCount",  TOUCH_FN(getButtonsCount));
	scriptEnvironment.addModuleFunction("Touch", "getAxesCount",     TOUCH_FN(getAxesCount));
	scriptEnvironment.addModuleFunction("Touch", "getIsPressed",      TOUCH_FN(getIsPressed));
	scriptEnvironment.addModuleFunction("Touch", "getIsReleased",     TOUCH_FN(getIsReleased));
	scriptEnvironment.addModuleFunction("Touch", "getIsAnyPressed",  TOUCH_FN(getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Touch", "getIsAnyReleased", TOUCH_FN(getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Touch", "getAxis",         TOUCH_FN(getAxis));
	scriptEnvironment.addModuleFunction("Touch", "getButtonName",  TOUCH_FN(getButtonName));
	scriptEnvironment.addModuleFunction("Touch", "getAxisName",    TOUCH_FN(getAxisName));
	scriptEnvironment.addModuleFunction("Touch", "getButtonId",    TOUCH_FN(getButtonId));
	scriptEnvironment.addModuleFunction("Touch", "getAxisId",      TOUCH_FN(getAxisId));

	scriptEnvironment.addModuleFunction("Pad1", "getName",         JOYPAD_FN(0, getName));
	scriptEnvironment.addModuleFunction("Pad1", "getIsConnected",    JOYPAD_FN(0, getIsConnected));
	scriptEnvironment.addModuleFunction("Pad1", "getButtonsCount",  JOYPAD_FN(0, getButtonsCount));
	scriptEnvironment.addModuleFunction("Pad1", "getAxesCount",     JOYPAD_FN(0, getAxesCount));
	scriptEnvironment.addModuleFunction("Pad1", "getIsPressed",      JOYPAD_FN(0, getIsPressed));
	scriptEnvironment.addModuleFunction("Pad1", "getIsReleased",     JOYPAD_FN(0, getIsReleased));
	scriptEnvironment.addModuleFunction("Pad1", "getIsAnyPressed",  JOYPAD_FN(0, getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Pad1", "getIsAnyReleased", JOYPAD_FN(0, getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Pad1", "getAxis",         JOYPAD_FN(0, getAxis));
	scriptEnvironment.addModuleFunction("Pad1", "getButtonName",  JOYPAD_FN(0, getButtonName));
	scriptEnvironment.addModuleFunction("Pad1", "getAxisName",    JOYPAD_FN(0, getAxisName));
	scriptEnvironment.addModuleFunction("Pad1", "getButtonId",    JOYPAD_FN(0, getButtonId));
	scriptEnvironment.addModuleFunction("Pad1", "getAxisId",      JOYPAD_FN(0, getAxisId));

	scriptEnvironment.addModuleFunction("Pad2", "getName", JOYPAD_FN(1, getName));
	scriptEnvironment.addModuleFunction("Pad2", "getIsConnected",    JOYPAD_FN(1, getIsConnected));
	scriptEnvironment.addModuleFunction("Pad2", "getButtonsCount",  JOYPAD_FN(1, getButtonsCount));
	scriptEnvironment.addModuleFunction("Pad2", "getAxesCount",     JOYPAD_FN(1, getAxesCount));
	scriptEnvironment.addModuleFunction("Pad2", "getIsPressed",      JOYPAD_FN(1, getIsPressed));
	scriptEnvironment.addModuleFunction("Pad2", "getIsReleased",     JOYPAD_FN(1, getIsReleased));
	scriptEnvironment.addModuleFunction("Pad2", "getIsAnyPressed",  JOYPAD_FN(1, getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Pad2", "getIsAnyReleased", JOYPAD_FN(1, getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Pad2", "getAxis",         JOYPAD_FN(1, getAxis));
	scriptEnvironment.addModuleFunction("Pad2", "getButtonName",  JOYPAD_FN(1, getButtonName));
	scriptEnvironment.addModuleFunction("Pad2", "getAxisName",    JOYPAD_FN(1, getAxisName));
	scriptEnvironment.addModuleFunction("Pad2", "getButtonId",    JOYPAD_FN(1, getButtonId));
	scriptEnvironment.addModuleFunction("Pad2", "getAxisId",      JOYPAD_FN(1, getAxisId));

	scriptEnvironment.addModuleFunction("Pad3", "getName", JOYPAD_FN(2, getName));
	scriptEnvironment.addModuleFunction("Pad3", "getIsConnected",    JOYPAD_FN(2, getIsConnected));
	scriptEnvironment.addModuleFunction("Pad3", "getButtonsCount",  JOYPAD_FN(2, getButtonsCount));
	scriptEnvironment.addModuleFunction("Pad3", "getAxesCount",     JOYPAD_FN(2, getAxesCount));
	scriptEnvironment.addModuleFunction("Pad3", "getIsPressed",      JOYPAD_FN(2, getIsPressed));
	scriptEnvironment.addModuleFunction("Pad3", "getIsReleased",     JOYPAD_FN(2, getIsReleased));
	scriptEnvironment.addModuleFunction("Pad3", "getIsAnyPressed",  JOYPAD_FN(2, getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Pad3", "getIsAnyReleased", JOYPAD_FN(2, getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Pad3", "getAxis",         JOYPAD_FN(2, getAxis));
	scriptEnvironment.addModuleFunction("Pad3", "getButtonName",  JOYPAD_FN(2, getButtonName));
	scriptEnvironment.addModuleFunction("Pad3", "getAxisName",    JOYPAD_FN(2, getAxisName));
	scriptEnvironment.addModuleFunction("Pad3", "getButtonId",    JOYPAD_FN(2, getButtonId));
	scriptEnvironment.addModuleFunction("Pad3", "getAxisId",      JOYPAD_FN(2, getAxisId));

	scriptEnvironment.addModuleFunction("Pad4", "getName", JOYPAD_FN(3, getName));
	scriptEnvironment.addModuleFunction("Pad4", "getIsConnected",    JOYPAD_FN(3, getIsConnected));
	scriptEnvironment.addModuleFunction("Pad4", "getButtonsCount",  JOYPAD_FN(3, getButtonsCount));
	scriptEnvironment.addModuleFunction("Pad4", "getAxesCount",     JOYPAD_FN(3, getAxesCount));
	scriptEnvironment.addModuleFunction("Pad4", "getIsPressed",      JOYPAD_FN(3, getIsPressed));
	scriptEnvironment.addModuleFunction("Pad4", "getIsReleased",     JOYPAD_FN(3, getIsReleased));
	scriptEnvironment.addModuleFunction("Pad4", "getIsAnyPressed",  JOYPAD_FN(3, getIsAnyPressed));
	scriptEnvironment.addModuleFunction("Pad4", "getIsAnyReleased", JOYPAD_FN(3, getIsAnyReleased));
	scriptEnvironment.addModuleFunction("Pad4", "getAxis",         JOYPAD_FN(3, getAxis));
	scriptEnvironment.addModuleFunction("Pad4", "getButtonName",  JOYPAD_FN(3, getButtonName));
	scriptEnvironment.addModuleFunction("Pad4", "getAxisName",    JOYPAD_FN(3, getAxisName));
	scriptEnvironment.addModuleFunction("Pad4", "getButtonId",    JOYPAD_FN(3, getButtonId));
	scriptEnvironment.addModuleFunction("Pad4", "getAxisId",      JOYPAD_FN(3, getAxisId));

	scriptEnvironment.addModuleFunction("World", "spawnUnit", world_spawnUnit);
	scriptEnvironment.addModuleFunction("World", "spawnEmptyUnit", world_spawnEmptyUnit);
	scriptEnvironment.addModuleFunction("World", "destroyUnit", world_destroyUnit);
	scriptEnvironment.addModuleFunction("World", "getUnitListCount", world_getUnitListCount);
	scriptEnvironment.addModuleFunction("World", "getUnitList", world_getUnitList);
	scriptEnvironment.addModuleFunction("World", "getCamera", world_getCamera);
	scriptEnvironment.addModuleFunction("World", "setCameraProjectionType", world_setCameraProjectionType);
	scriptEnvironment.addModuleFunction("World", "getCameraProjectionType", world_getCameraProjectionType);
	scriptEnvironment.addModuleFunction("World", "getCameraFov", world_getCameraFov);
	scriptEnvironment.addModuleFunction("World", "setCameraFov", world_setCameraFov);
	scriptEnvironment.addModuleFunction("World", "getCameraAspect", world_getCameraAspect);
	scriptEnvironment.addModuleFunction("World", "setCameraAspect", world_setCameraAspect);
	scriptEnvironment.addModuleFunction("World", "getCameraNearClipDistance", world_getCameraNearClipDistance);
	scriptEnvironment.addModuleFunction("World", "setCameraNearClipDistance", world_setCameraNearClipDistance);
	scriptEnvironment.addModuleFunction("World", "getCameraFarClipDistance", world_getCameraFarClipDistance);
	scriptEnvironment.addModuleFunction("World", "setCameraFarClipDistance", world_setCameraFarClipDistance);
	scriptEnvironment.addModuleFunction("World", "setCameraOrthographicMetrics", world_setCameraOrthographicMetrics);
	scriptEnvironment.addModuleFunction("World", "getCameraWorldFromScreen", world_getCameraWorldFromScreen);
	scriptEnvironment.addModuleFunction("World", "getCameraScreenFromWorld", world_getCameraScreenFromWorld);
	scriptEnvironment.addModuleFunction("World", "updateAnimations", world_updateAnimations);
	scriptEnvironment.addModuleFunction("World", "updateScene", world_updateScene);
	scriptEnvironment.addModuleFunction("World", "update", world_update);
	scriptEnvironment.addModuleFunction("World", "playSound", world_playSound);
	scriptEnvironment.addModuleFunction("World", "stopSound", world_stopSound);
	scriptEnvironment.addModuleFunction("World", "linkSound", world_linkSound);
	scriptEnvironment.addModuleFunction("World", "setListenerPose", world_setListenerPose);
	scriptEnvironment.addModuleFunction("World", "setSoundPosition", world_setSoundPosition);
	scriptEnvironment.addModuleFunction("World", "setSoundRange", world_setSoundRange);
	scriptEnvironment.addModuleFunction("World", "setSoundVolume", world_setSoundVolume);
	scriptEnvironment.addModuleFunction("World", "createDebugLine", world_createDebugLine);
	scriptEnvironment.addModuleFunction("World", "destroyDebugLine", world_destroyDebugLine);
	scriptEnvironment.addModuleFunction("World", "createScreenGui", world_createScreenGui);
	scriptEnvironment.addModuleFunction("World", "destroyGui", world_destroyGui);
	scriptEnvironment.addModuleFunction("World", "loadLevel", world_loadLevel);
	scriptEnvironment.addModuleFunction("World", "getSceneGraph", world_getSceneGraph);
	scriptEnvironment.addModuleFunction("World", "getRenderWorld", world_getRenderWorld);
	scriptEnvironment.addModuleFunction("World", "getPhysicsWorld", world_getPhysicsWorld);
	scriptEnvironment.addModuleFunction("World", "getSoundWorld", world_getSoundWorld);
	scriptEnvironment.addModuleFunction("World", "__getIndex", "World");
	scriptEnvironment.addModuleFunction("World", "__getString", world_getString);

	scriptEnvironment.addModuleFunction("SceneGraph", "create", sceneGraph_create);
	scriptEnvironment.addModuleFunction("SceneGraph", "destroy", sceneGraph_destroy);
	scriptEnvironment.addModuleFunction("SceneGraph", "getTransformInstanceList", sceneGraph_getTransformInstanceList);
	scriptEnvironment.addModuleFunction("SceneGraph", "getLocalPosition", sceneGraph_getLocalPosition);
	scriptEnvironment.addModuleFunction("SceneGraph", "getLocalRotation", sceneGraph_getLocalRotation);
	scriptEnvironment.addModuleFunction("SceneGraph", "getLocalScale", sceneGraph_getLocalScale);
	scriptEnvironment.addModuleFunction("SceneGraph", "getLocalPose", sceneGraph_getLocalPose);
	scriptEnvironment.addModuleFunction("SceneGraph", "getWorldPosition", sceneGraph_getWorldPosition);
	scriptEnvironment.addModuleFunction("SceneGraph", "getWorldRotation", sceneGraph_getWorldRotation);
	scriptEnvironment.addModuleFunction("SceneGraph", "getWorldPose", sceneGraph_getWorldPose);
	scriptEnvironment.addModuleFunction("SceneGraph", "setLocalPosition", sceneGraph_setLocalPosition);
	scriptEnvironment.addModuleFunction("SceneGraph", "setLocalRotation", sceneGraph_setLocalRotation);
	scriptEnvironment.addModuleFunction("SceneGraph", "setLocalScale", sceneGraph_setLocalScale);
	scriptEnvironment.addModuleFunction("SceneGraph", "setLocalPose", sceneGraph_setLocalPose);
	scriptEnvironment.addModuleFunction("SceneGraph", "link", sceneGraph_link);
	scriptEnvironment.addModuleFunction("SceneGraph", "unlink", sceneGraph_unlink);

	scriptEnvironment.addModuleFunction("UnitManager", "create", unitManager_create);
	scriptEnvironment.addModuleFunction("UnitManager", "getIsAlive", unitManager_getIsAlive);

	scriptEnvironment.addModuleFunction("RenderWorld", "createMesh", renderWorld_createMesh);
	scriptEnvironment.addModuleFunction("RenderWorld", "destroyMesh", renderWorld_destroyMesh);
	scriptEnvironment.addModuleFunction("RenderWorld", "getMeshInstanceList", renderWorld_getMeshInstanceList);
	scriptEnvironment.addModuleFunction("RenderWorld", "getMeshObb", renderWorld_getMeshObb);
	scriptEnvironment.addModuleFunction("RenderWorld", "getMeshRaycast", renderWorld_getMeshRaycast);
	scriptEnvironment.addModuleFunction("RenderWorld", "setMeshVisible", renderWorld_setMeshVisible);
	scriptEnvironment.addModuleFunction("RenderWorld", "createSprite", renderWorld_createSprite);
	scriptEnvironment.addModuleFunction("RenderWorld", "destroySprite", renderWorld_destroySprite);
	scriptEnvironment.addModuleFunction("RenderWorld", "getSpriteInstanceList", renderWorld_getSpriteInstanceList);
	scriptEnvironment.addModuleFunction("RenderWorld", "setSpriteFrame", renderWorld_setSpriteFrame);
	scriptEnvironment.addModuleFunction("RenderWorld", "setSpriteVisible", renderWorld_setSpriteVisible);
	scriptEnvironment.addModuleFunction("RenderWorld", "createLight", renderWorld_createLight);
	scriptEnvironment.addModuleFunction("RenderWorld", "destroyLight", renderWorld_destroyLight);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightInstanceList", renderWorld_getLightInstanceList);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightType", renderWorld_getLightType);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightColor", renderWorld_getLightColor);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightRange", renderWorld_getLightRange);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightIntensity", renderWorld_getLightIntensity);
	scriptEnvironment.addModuleFunction("RenderWorld", "getLightSpotAngle", renderWorld_getLightSpotAngle);
	scriptEnvironment.addModuleFunction("RenderWorld", "setLightType", renderWorld_setLightType);
	scriptEnvironment.addModuleFunction("RenderWorld", "setLightColor", renderWorld_setLightColor);
	scriptEnvironment.addModuleFunction("RenderWorld", "setLightRange", renderWorld_setLightRange);
	scriptEnvironment.addModuleFunction("RenderWorld", "setLightIntensity", renderWorld_setLightIntensity);
	scriptEnvironment.addModuleFunction("RenderWorld", "setLightSpotAngle", renderWorld_setLightSpotAngle);
	scriptEnvironment.addModuleFunction("RenderWorld", "enableDebugDrawing", renderWorld_enableDebugDrawing);

	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorInstanceList", physicsWorld_getActorInstanceList);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorWorldPosition", physicsWorld_getActorWorldPosition);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorWorldRotation", physicsWorld_getActorWorldRotation);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorWorldPose", physicsWorld_getActorWorldPose);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "teleportActorWorldPosition", physicsWorld_teleportActorWorldPosition);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "teleportActorWorldRotation", physicsWorld_teleportActorWorldRotation);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "teleportActorWorldPose", physicsWorld_teleportActorWorldPose);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorCenterOfMass", physicsWorld_getActorCenterOfMass);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "enableActorGravity", physicsWorld_enableActorGravity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "disableActorGravity", physicsWorld_disableActorGravity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "enableActorCollision", physicsWorld_enableActorCollision);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "disableActorCollision", physicsWorld_disableActorCollision);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorCollisionFilter", physicsWorld_setActorCollisionFilter);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorKinematic", physicsWorld_setActorKinematic);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "moveActor", physicsWorld_moveActor);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getIsStatic", physicsWorld_getIsStatic);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getIsDynamic", physicsWorld_getIsDynamic);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getIsKinematic", physicsWorld_getIsKinematic);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getIsNonKinematic", physicsWorld_getIsNonKinematic);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorLinearDamping", physicsWorld_getActorLinearDamping);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorLinearDamping", physicsWorld_setActorLinearDamping);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorAngularDamping", physicsWorld_getActorAngularDamping);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorAngularDamping", physicsWorld_setActorAngularDamping);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorLinearVelocity", physicsWorld_getActorLinearVelocity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorLinearVelocity", physicsWorld_setActorLinearVelocity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getActorAngularVelocity", physicsWorld_getActorAngularVelocity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setActorAngularVelocity", physicsWorld_setActorAngularVelocity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "addActorImpulse", physicsWorld_addActorImpulse);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "addActorImpulseAt", physicsWorld_addActorImpulseAt);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "addActorTorqueImpulse", physicsWorld_addActorTorqueImpulse);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "pushActor", physicsWorld_pushActor);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "pushActorAt", physicsWorld_pushActorAt);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getIsSleeping", physicsWorld_getIsSleeping);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "wakeUp", physicsWorld_wakeUp);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getControllerInstanceList", physicsWorld_getControllerInstanceList);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "moveController", physicsWorld_moveController);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "createJoint", physicsWorld_createJoint);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "getGravity", physicsWorld_getGravity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "setGravity", physicsWorld_setGravity);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "raycast", physicsWorld_raycast);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "enableDebugDrawing", physicsWorld_enableDebugDrawing);
	scriptEnvironment.addModuleFunction("PhysicsWorld", "__getIndex", "PhysicsWorld");
	scriptEnvironment.addModuleFunction("PhysicsWorld", "__getString", physicsWorld_getString);

	scriptEnvironment.addModuleFunction("SoundWorld", "stopAll", soundWorld_stopAll);
	scriptEnvironment.addModuleFunction("SoundWorld", "pauseAll", soundWorld_pauseAll);
	scriptEnvironment.addModuleFunction("SoundWorld", "resumeAll", soundWorld_resumeAll);
	scriptEnvironment.addModuleFunction("SoundWorld", "getIsPlaying", soundWorld_getIsPlaying);
	scriptEnvironment.addModuleFunction("SoundWorld", "__getIndex", "SoundWorld");
	scriptEnvironment.addModuleFunction("SoundWorld", "__getString", soundWorld_getString);

	scriptEnvironment.addModuleFunction("Device", "getArgumentList", device_getArgumentList);
	scriptEnvironment.addModuleFunction("Device", "getPlatform", device_getPlatform);
	scriptEnvironment.addModuleFunction("Device", "getArchitecture", device_getArchitecture);
	scriptEnvironment.addModuleFunction("Device", "getVersion", device_getVersion);
	scriptEnvironment.addModuleFunction("Device", "getLastDeltaTime", device_getLastDeltaTime);
	scriptEnvironment.addModuleFunction("Device", "quit", device_quit);
	scriptEnvironment.addModuleFunction("Device", "getResolution", device_getResolution);
	scriptEnvironment.addModuleFunction("Device", "createWorld", device_createWorld);
	scriptEnvironment.addModuleFunction("Device", "destroyWorld", device_destroyWorld);
	scriptEnvironment.addModuleFunction("Device", "render", device_render);
	scriptEnvironment.addModuleFunction("Device", "createResourcePackage", device_createResourcePackage);
	scriptEnvironment.addModuleFunction("Device", "destroyResourcePackage", device_destroyResourcePackage);
	scriptEnvironment.addModuleFunction("Device", "sendWithConsoleServer", device_sendWithConsoleServer);
	scriptEnvironment.addModuleFunction("Device", "canGet", device_canGet);
	scriptEnvironment.addModuleFunction("Device", "enable_resource_autoload", device_enableResourceAutoload);
	scriptEnvironment.addModuleFunction("Device", "getTemporaryObjectsCount", device_getTemporaryObjectsCount);
	scriptEnvironment.addModuleFunction("Device", "setTemporaryObjectsCount", device_setTemporaryObjectsCount);
	scriptEnvironment.addModuleFunction("Device", "getGuid", device_getGuid);

	scriptEnvironment.addModuleFunction("Profiler", "enterProfileScope", profiler_enterProfileScope);
	scriptEnvironment.addModuleFunction("Profiler", "leaveProfileScope", profiler_leaveProfileScope);
	scriptEnvironment.addModuleFunction("Profiler", "record", profiler_record);

	scriptEnvironment.addModuleFunction("DebugLine", "addLine", debugLine_addLine);
	scriptEnvironment.addModuleFunction("DebugLine", "addAxes", debugLine_addAxes);
	scriptEnvironment.addModuleFunction("DebugLine", "addCircle", debugLine_addCircle);
	scriptEnvironment.addModuleFunction("DebugLine", "addCone", debugLine_addCone);
	scriptEnvironment.addModuleFunction("DebugLine", "addSphere", debugLine_addSphere);
	scriptEnvironment.addModuleFunction("DebugLine", "addObb", debugLine_addObb);
	scriptEnvironment.addModuleFunction("DebugLine", "addUnit", debugLine_addUnit);
	scriptEnvironment.addModuleFunction("DebugLine", "reset", debugLine_reset);
	scriptEnvironment.addModuleFunction("DebugLine", "submit", debugLine_submit);
	scriptEnvironment.addModuleFunction("DebugLine", "__getIndex", "DebugLine");
	scriptEnvironment.addModuleFunction("DebugLine", "__getString", debugLine_toString);

	scriptEnvironment.addModuleFunction("ResourcePackage", "load", resourcePackage_load);
	scriptEnvironment.addModuleFunction("ResourcePackage", "unload", resourcePackage_unload);
	scriptEnvironment.addModuleFunction("ResourcePackage", "flush", resourcePackage_flush);
	scriptEnvironment.addModuleFunction("ResourcePackage", "hasLoaded", resourcePackage_hasLoaded);
	scriptEnvironment.addModuleFunction("ResourcePackage", "__getIndex", "ResourcePackage");
	scriptEnvironment.addModuleFunction("ResourcePackage", "__getString", resourcePackage_getString);

	scriptEnvironment.addModuleFunction("Material", "setFloat", material_setFloat);
	scriptEnvironment.addModuleFunction("Material", "setVector2", material_setVector2);
	scriptEnvironment.addModuleFunction("Material", "setVector3", material_setVector3);

	scriptEnvironment.addModuleFunction("Gui", "getResolution", gui_getResolution);
	scriptEnvironment.addModuleFunction("Gui", "move", gui_move);
	scriptEnvironment.addModuleFunction("Gui", "getGuiFromScreen", gui_getGuiFromScreen);
	scriptEnvironment.addModuleFunction("Gui", "drawRectangle", gui_drawRectangle);
	scriptEnvironment.addModuleFunction("Gui", "drawImage", gui_drawImage);
	scriptEnvironment.addModuleFunction("Gui", "drawImageUv", gui_drawImageUv);
	scriptEnvironment.addModuleFunction("Gui", "drawText", gui_drawText);

	scriptEnvironment.addModuleFunction("Display", "getModes", display_getModes);
	scriptEnvironment.addModuleFunction("Display", "setMode", display_setMode);

	scriptEnvironment.addModuleFunction("Window", "show", window_show);
	scriptEnvironment.addModuleFunction("Window", "hide", window_hide);
	scriptEnvironment.addModuleFunction("Window", "resize", window_resize);
	scriptEnvironment.addModuleFunction("Window", "move", window_move);
	scriptEnvironment.addModuleFunction("Window", "minimize", window_minimize);
	scriptEnvironment.addModuleFunction("Window", "restore", window_restore);
	scriptEnvironment.addModuleFunction("Window", "getTitle", window_getTitle);
	scriptEnvironment.addModuleFunction("Window", "setTitle",  window_setTitle);
	scriptEnvironment.addModuleFunction("Window", "setShowCursor", window_showCursor);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka