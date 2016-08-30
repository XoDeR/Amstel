// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

struct DebugLine
{
private:
	uint32_t marker = 0;
public:
	// Default number of segments
	static const uint32_t NUM_SEGMENTS = 36;
	static const uint32_t MAX_LINES = 32768;

	struct Line
	{
		Vector3 p0;
		uint32_t c0;
		Vector3 p1;
		uint32_t c1;
	};

	// Whether to enable <depthTest>
	explicit DebugLine(bool depthTest);
	~DebugLine();

	// Adds a line from <start> to <end> with the given <color>
	void addLine(const Vector3& start, const Vector3& end, const Color4& color);
	// Adds lines for each axis with the given <length>
	void addAxes(const Matrix4x4& m, float length = 1.0f);
	// Adds a circle at <center> with the given <radius> and <normal> vector
	void addCircle(const Vector3& center, float radius, const Vector3& normal, const Color4& color, uint32_t segments = NUM_SEGMENTS);
	// Adds a cone with the base centered at <from> and the tip at <to>
	void addCone(const Vector3& from, const Vector3& to, float radius, const Color4& color, uint32_t segments = NUM_SEGMENTS);
	// Adds a sphere at <center> with the given <radius> and <color>
	void addSphere(const Vector3& center, const float radius, const Color4& color, uint32_t segments = NUM_SEGMENTS);
	// Adds an oriented bounding box
	// <transformMatrix> describes the position and orientation of the box
	// halfExtents describes the size of the box along the axis
	void addObb(const Matrix4x4& transformMatrix, const Vector3& halfExtents, const Color4& color);
	// Adds the mesh described by (vertices, stride, indices, num)
	void addMesh(const Matrix4x4& transformMatrix, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t indicesCount, const Color4& color);
	// Adds the meshes from the unit <name>
	void addUnit(ResourceManager& resourceManager, const Matrix4x4& transformMatrix, StringId64 name, const Color4& color);
	// Resets all the lines
	void reset();
	// Submits the lines to renderer for drawing
	void submit();

	StringId32 shaderNameHash;
	bgfx::VertexDecl vertexDecl;

	uint32_t debugLinesCount = 0;
	Line lineList[MAX_LINES];
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka