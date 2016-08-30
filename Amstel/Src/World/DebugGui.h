#pragma once

#include "Core/Math/MathTypes.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

#include <bgfx/bgfx.h>

namespace Rio
{

// Immediate mode GUI
struct DebugGui
{
private:
	uint32_t marker = 0;
public:

	struct VertexData
	{
		Vector3 position;
		Vector2 uv;
		uint32_t color;
	};

	struct IndexData
	{
		uint16_t a;
		uint16_t b;
	};

	DebugGui(ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, uint16_t width, uint16_t height);
	~DebugGui();

	Vector2 getResolution() const;
	void move(const Vector2& position);

	Vector2 getGuiFromScreen(const Vector2& position);

	void drawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, StringId64 material, const Color4& color);
	void drawRectangle3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawRectangle(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawImageUv3D(const Vector3& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void drawImageUv(const Vector2& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void drawImage3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawImage(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawText3D(const Vector3& position, uint32_t fontSize, const char* text, StringId64 font, StringId64 material, const Color4& color);
	void drawText(const Vector2& position, uint32_t fontSize, const char* text, StringId64 font, StringId64 material, const Color4& color);

	ResourceManager* resourceManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	uint16_t width;
	uint16_t height;
	Matrix4x4 projectionTransformMatrix = MATRIX4X4_IDENTITY;
	Matrix4x4 worldTransformMatrix = MATRIX4X4_IDENTITY;
	bgfx::VertexDecl positionTextureCoordColorVertexDecl;
};

} // namespace Rio
