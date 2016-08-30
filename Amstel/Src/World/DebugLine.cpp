// Copyright (c) 2016 Volodymyr Syvochka
#include "World/DebugLine.h"

#include "Core/Math/Color4.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

#include "Device/Device.h"

#include "Resource/MeshResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/UnitResource.h"

#include "World/ShaderManager.h"

#include <string.h> // memcpy

namespace Rio
{

DebugLine::DebugLine(bool depthTest)
	: marker(DEBUG_LINE_MARKER)
	, shaderNameHash(depthTest ? "debugLine" : "debugLineNoDepthTest")
{
	vertexDecl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();
}

DebugLine::~DebugLine()
{
	marker = 0;
}

void DebugLine::addLine(const Vector3& start, const Vector3& end, const Color4& color)
{
	if (debugLinesCount >= MAX_LINES)
	{
		return;
	}

	lineList[debugLinesCount].p0 = start;
	lineList[debugLinesCount].c0 = getAbgr(color);
	lineList[debugLinesCount].p1 = end;
	lineList[debugLinesCount].c1 = getAbgr(color);

	++debugLinesCount;
}

void DebugLine::addAxes(const Matrix4x4& m, float length)
{
	const Vector3 position = getTranslation(m);
	addLine(position, position + getAxisX(m)*length, COLOR4_RED);
	addLine(position, position + getAxisY(m)*length, COLOR4_GREEN);
	addLine(position, position + getAxisZ(m)*length, COLOR4_BLUE);
}

void DebugLine::addCircle(const Vector3& center, float radius, const Vector3& normal, const Color4& color, uint32_t segments)
{
	const Vector3 direction = normal;
	const Vector3 pointList[] =
	{
		{ direction.z, direction.z, -direction.x - direction.y },
		{ -direction.y - direction.z, direction.x, direction.x }
	};
	const int index = ((direction.z != 0.0f) && (-direction.x != direction.y));
	Vector3 right = pointList[index];
	normalize(right);

	const float angleIncrement = 360.0f / (float)(segments >= 3 ? segments : 3);
	float deg0 = 0.0f;
	for (uint32_t segmentIndex = 0; segmentIndex < segments; ++segmentIndex, deg0 += angleIncrement)
	{
		const float rad0 = getRadiansFromDegrees(deg0);
		const float rad1 = getRadiansFromDegrees(deg0 + angleIncrement);

		const Vector3 from0 = right*cos(-rad0) + cross(direction, right)*sin(-rad0) + direction*dot(direction, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(direction, right)*sin(-rad1) + direction*dot(direction, right)*(1.0f-cos(-rad1));

		addLine(center + radius*from0, center + radius*from1, color);
	}
}

void DebugLine::addCone(const Vector3& from, const Vector3& to, float radius, const Color4& color, uint32_t segments)
{
	Vector3 direction = to - from;
	Rio::normalize(direction);
	const Vector3 pointList[] =
	{
		{ direction.z, direction.z, -direction.x - direction.y },
		{ -direction.y - direction.z, direction.x, direction.x }
	};
	const int idx = ((direction.z != 0.0f) && (-direction.x != direction.y));
	Vector3 right = pointList[idx];
	Rio::normalize(right);

	const float angleIncrement = 360.0f / (float)(segments >= 3 ? segments : 3);
	float deg0 = 0.0f;
	for (uint32_t segmentIndex = 0; segmentIndex < segments; ++segmentIndex, deg0 += angleIncrement)
	{
		const float rad0 = getRadiansFromDegrees(deg0);
		const float rad1 = getRadiansFromDegrees(deg0 + angleIncrement);

		const Vector3 from0 = right*cos(-rad0) + cross(direction, right)*sin(-rad0) + direction*dot(direction, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(direction, right)*sin(-rad1) + direction*dot(direction, right)*(1.0f-cos(-rad1));

		addLine(from + radius*from0, to, color);
		addLine(from + radius*from0, from + radius*from1, color);
	}
}

void DebugLine::addSphere(const Vector3& center, const float radius, const Color4& color, uint32_t segments)
{
	addCircle(center, radius, VECTOR3_XAXIS, color, segments);
	addCircle(center, radius, VECTOR3_YAXIS, color, segments);
	addCircle(center, radius, VECTOR3_ZAXIS, color, segments);
}

void DebugLine::addObb(const Matrix4x4& transformMatrix, const Vector3& halfExtents, const Color4& color)
{
	const Vector3 o = createVector3(transformMatrix.t.x, transformMatrix.t.y, transformMatrix.t.z);
	const Vector3 x = createVector3(transformMatrix.x.x, transformMatrix.x.y, transformMatrix.x.z) * halfExtents.x;
	const Vector3 y = createVector3(transformMatrix.y.x, transformMatrix.y.y, transformMatrix.y.z) * halfExtents.y;
	const Vector3 z = createVector3(transformMatrix.z.x, transformMatrix.z.y, transformMatrix.z.z) * halfExtents.z;

	// Back face
	addLine(o - x - y - z, o + x - y - z, color);
	addLine(o + x - y - z, o + x + y - z, color);
	addLine(o + x + y - z, o - x + y - z, color);
	addLine(o - x + y - z, o - x - y - z, color);

	addLine(o - x - y + z, o + x - y + z, color);
	addLine(o + x - y + z, o + x + y + z, color);
	addLine(o + x + y + z, o - x + y + z, color);
	addLine(o - x + y + z, o - x - y + z, color);

	addLine(o - x - y - z, o - x - y + z, color);
	addLine(o + x - y - z, o + x - y + z, color);
	addLine(o + x + y - z, o + x + y + z, color);
	addLine(o - x + y - z, o - x + y + z, color);
}

void DebugLine::addMesh(const Matrix4x4& transformMatrix, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t indicesCount, const Color4& color)
{
	for (uint32_t i = 0; i < indicesCount; i += 3)
	{
		const uint32_t i0 = indices[i + 0];
		const uint32_t i1 = indices[i + 1];
		const uint32_t i2 = indices[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertices + i0*stride) * transformMatrix;
		const Vector3& v1 = *(const Vector3*)((const char*)vertices + i1*stride) * transformMatrix;
		const Vector3& v2 = *(const Vector3*)((const char*)vertices + i2*stride) * transformMatrix;

		addLine(v0, v1, color);
		addLine(v1, v2, color);
		addLine(v2, v0, color);
	}
}

void DebugLine::addUnit(ResourceManager& resourceManager, const Matrix4x4& transformMatrix, StringId64 name, const Color4& color)
{
	const UnitResource& unitResource = *(const UnitResource*)resourceManager.get(RESOURCE_TYPE_UNIT, name);

	const char* componentData = (const char*)(&unitResource + 1);

	for (uint32_t componentTypesIndex = 0; componentTypesIndex < unitResource.componentTypesCount; ++componentTypesIndex)
	{
		const ComponentData* component = (const ComponentData*)componentData;
		const uint32_t* unitIndexList = (const uint32_t*)(component + 1);
		const char* data = (const char*)(unitIndexList + component->instancesCount);

		if (component->type == COMPONENT_TYPE_MESH_RENDERER)
		{
			const MeshRendererDesc* meshRendererDesc = (const MeshRendererDesc*)data;
			for (uint32_t i = 0; i < component->instancesCount; ++i, ++meshRendererDesc)
			{
				const MeshResource* meshResource = (const MeshResource*)resourceManager.get(RESOURCE_TYPE_MESH, meshRendererDesc->meshResource);
				const MeshGeometry* meshGeometry = meshResource->getMeshGeometry(meshRendererDesc->geometryName);

				addMesh(transformMatrix
					, meshGeometry->vertices.data
					, meshGeometry->vertices.stride
					, (uint16_t*)meshGeometry->indices.data
					, meshGeometry->indices.indicesCount
					, color
					);
			}
		}

		componentData += component->size + sizeof(ComponentData);
	}
}

void DebugLine::reset()
{
	debugLinesCount = 0;
}

void DebugLine::submit()
{
	if (debugLinesCount == 0)
	{
		return;
	}

	if (!checkAvailTransientVertexBuffer(debugLinesCount * 2, vertexDecl))
	{
		return;
	}

	bgfx::TransientVertexBuffer bgfxTransientVertexBuffer;
	bgfx::allocTransientVertexBuffer(&bgfxTransientVertexBuffer, debugLinesCount * 2, vertexDecl);
	memcpy(bgfxTransientVertexBuffer.data, lineList, sizeof(Line) * debugLinesCount);

	const ShaderData& shaderData = getDevice()->getShaderManager()->get(shaderNameHash);

	bgfx::setVertexBuffer(&bgfxTransientVertexBuffer, 0, debugLinesCount * 2);
	bgfx::setState(shaderData.state);
	bgfx::submit(1, shaderData.bgfxProgramHandle);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka