#include "World/Gui.h"
#include "Core/Math/Color4.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Strings/Utf8.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Resource/FontResource.h"
#include "Resource/MaterialResource.h"
#include "Resource/ResourceManager.h"
#include "World/MaterialManager.h"

#include <bgfx/bgfx.h>

namespace Rio
{

Gui::Gui(ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, uint16_t width, uint16_t height)
	: resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, width(width)
	, height(height)
{
	setToOrthographic(projectionTransformMatrix, 0, width, 0, height, -0.01f, 100.0f);

	positionTextureCoordColorVertexDecl
		.begin()
		.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
		.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
		.end();
}

Vector2 Gui::getResolution() const
{
	return createVector2(width, height);
}

void Gui::move(const Vector2& position)
{
	setToIdentity(worldTransformMatrix);
	setTranslation(worldTransformMatrix, createVector3(position.x, position.y, 0));
}

Vector2 Gui::getGuiFromScreen(const Vector2& position)
{
	return createVector2(position.x, height - position.y);
}

void Gui::drawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer transientVertexBuffer;
	bgfx::TransientIndexBuffer transientIndexBuffer;
	bgfx::allocTransientVertexBuffer(&transientVertexBuffer, 3, positionTextureCoordColorVertexDecl);
	bgfx::allocTransientIndexBuffer(&transientIndexBuffer, 3);

	VertexData* vertexData = (VertexData*)transientVertexBuffer.data;
	vertexData[0].position.x = a.x;
	vertexData[0].position.y = a.y;
	vertexData[0].position.z = a.z;
	vertexData[0].uv.x = 0.0f;
	vertexData[0].uv.y = 0.0f;
	vertexData[0].color = getAbgr(color);

	vertexData[1].position.x = b.x;
	vertexData[1].position.y = b.y;
	vertexData[1].position.z = b.z;
	vertexData[1].uv.x = 1.0f;
	vertexData[1].uv.y = 0.0f;
	vertexData[1].color = getAbgr(color);

	vertexData[2].position.x = c.x;
	vertexData[2].position.y = c.y;
	vertexData[2].position.z = c.z;
	vertexData[2].uv.x = 1.0f;
	vertexData[2].uv.y = 1.0f;
	vertexData[2].color = getAbgr(color);

	uint16_t* indices = (uint16_t*)transientIndexBuffer.data;
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
}

void Gui::drawRectangle3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer transientVertexBuffer;
	bgfx::TransientIndexBuffer transientIndexBuffer;
	bgfx::allocTransientVertexBuffer(&transientVertexBuffer, 4, positionTextureCoordColorVertexDecl);
	bgfx::allocTransientIndexBuffer(&transientIndexBuffer, 6);

	VertexData* vd = (VertexData*)transientVertexBuffer.data;
	vd[0].position.x = position.x;
	vd[0].position.y = position.y;
	vd[0].position.z = position.z;
	vd[0].uv.x = 0.0f;
	vd[0].uv.y = 1.0f;
	vd[0].color = getAbgr(color);

	vd[1].position.x = position.x + size.x;
	vd[1].position.y = position.y;
	vd[1].position.z = position.z;
	vd[1].uv.x = 1.0f;
	vd[1].uv.y = 1.0f;
	vd[1].color = getAbgr(color);

	vd[2].position.x = position.x + size.x;
	vd[2].position.y = position.y + size.y;
	vd[2].position.z = position.z;
	vd[2].uv.x = 1.0f;
	vd[2].uv.y = 0.0f;
	vd[2].color = getAbgr(color);

	vd[3].position.x = position.x;
	vd[3].position.y = position.y + size.y;
	vd[3].position.z = position.z;
	vd[3].uv.x = 0.0f;
	vd[3].uv.y = 0.0f;
	vd[3].color = getAbgr(color);

	uint16_t* indices = (uint16_t*)transientIndexBuffer.data;
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	bgfx::setVertexBuffer(&transientVertexBuffer);
	bgfx::setIndexBuffer(&transientIndexBuffer);
	bgfx::setTransform(getFloatPointer(projectionTransformMatrix));

	materialManager->createMaterial(material);
	materialManager->get(material)->bind(*resourceManager, *shaderManager, 2);
}

void Gui::drawRectangle(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color)
{
	drawRectangle3D(createVector3(position.x, position.y, 0.0f), size, material, color);
}

void Gui::drawImageUv3D(const Vector3& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer transientVertexBuffer;
	bgfx::TransientIndexBuffer transientIndexBuffer;
	bgfx::allocTransientVertexBuffer(&transientVertexBuffer, 4, positionTextureCoordColorVertexDecl);
	bgfx::allocTransientIndexBuffer(&transientIndexBuffer, 6);

	VertexData* vd = (VertexData*)transientVertexBuffer.data;
	vd[0].position.x = position.x;
	vd[0].position.y = position.y;
	vd[0].position.z = position.z;
	vd[0].uv.x = 0.0f;
	vd[0].uv.y = 1.0f;
	vd[0].color = getAbgr(color);

	vd[1].position.x = position.x + size.x;
	vd[1].position.y = position.y;
	vd[1].position.z = position.z;
	vd[1].uv.x = 1.0f;
	vd[1].uv.y = 1.0f;
	vd[1].color = getAbgr(color);

	vd[2].position.x = position.x + size.x;
	vd[2].position.y = position.y + size.y;
	vd[2].position.z = position.z;
	vd[2].uv.x = 1.0f;
	vd[2].uv.y = 0.0f;
	vd[2].color = getAbgr(color);

	vd[3].position.x = position.x;
	vd[3].position.y = position.y + size.y;
	vd[3].position.z = position.z;
	vd[3].uv.x = 0.0f;
	vd[3].uv.y = 0.0f;
	vd[3].color = getAbgr(color);

	uint16_t* indices = (uint16_t*)transientIndexBuffer.data;
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	bgfx::setVertexBuffer(&transientVertexBuffer);
	bgfx::setIndexBuffer(&transientIndexBuffer);
}

void Gui::drawImageUv(const Vector2& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	drawImageUv3D(createVector3(position.x, position.y, 0.0f), size, uv0, uv1, material, color);
}

void Gui::drawImage3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color)
{
	drawImageUv3D(position, size, VECTOR2_ZERO, VECTOR2_ONE, material, color);
}

void Gui::drawImage(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color)
{
	drawImage3D(createVector3(position.x, position.y, 0.0f), size, material, color);
}

void Gui::drawText3D(const Vector3& position, uint32_t fontSize, const char* text, StringId64 font, StringId64 material, const Color4& color)
{
	const FontResource* fontResource = (FontResource*)resourceManager->get(RESOURCE_TYPE_FONT, font);
	const float scale = (float)fontSize / (float)fontResource->fontSize;
	const uint32_t length = getStringLength32(text);

	bgfx::TransientVertexBuffer transientVertexBuffer;
	bgfx::TransientIndexBuffer transientIndexBuffer;
	bgfx::allocTransientVertexBuffer(&transientVertexBuffer, 4 * length, positionTextureCoordColorVertexDecl);
	bgfx::allocTransientIndexBuffer(&transientIndexBuffer, 6 * length);

	uint16_t index = 0;
	float xPenAdvance = 0.0f;
	float yPenAdvance = 0.0f;

	Vector2 penPosition;

	uint32_t state = 0;
	uint32_t codePoint = 0;
	for (uint32_t i = 0; i < length; i++)
	{
		switch (text[i])
		{
			case '\n':
			{
				xPenAdvance = 0.0f;
				yPenAdvance -= fontResource->fontSize;
				continue;
			}
			case '\t':
			{
				xPenAdvance += fontSize * 4;
				continue;
			}
		}

		if (Utf8Fn::decode(&state, &codePoint, text[i]) == UTF8_ACCEPT)
		{
			const GlyphData& glyphData = *FontResourceFn::getGlyph(fontResource, codePoint);
			const float baseline = glyphData.height - glyphData.yOffset;

			// Set pen position
			penPosition.x = position.x + glyphData.xOffset;
			penPosition.y = position.y - baseline;

			// Position coords
			const float x0 = (penPosition.x + xPenAdvance) * scale;
			const float y0 = (penPosition.y + yPenAdvance) * scale;
			const float x1 = (penPosition.x + glyphData.width + xPenAdvance) * scale;
			const float y1 = (penPosition.y + glyphData.height + yPenAdvance) * scale;

			// Texture coords
			const float u0 = glyphData.x / fontResource->textureSize;
			const float v1 = glyphData.y / fontResource->textureSize; // Upper-left char corner
			const float u1 = u0 + glyphData.width / fontResource->textureSize;
			const float v0 = v1 + glyphData.height / fontResource->textureSize; // Bottom-left char corner

			// Fill vertex buffer
			VertexData* vertexData = (VertexData*)&transientVertexBuffer.data[i*4*sizeof(VertexData)];
			vertexData[0].position.x = x0;
			vertexData[0].position.y = y0;
			vertexData[0].position.z = position.z;
			vertexData[0].uv.x = u0;
			vertexData[0].uv.y = v0;
			vertexData[0].color = getAbgr(color);

			vertexData[1].position.x = x1;
			vertexData[1].position.y = y0;
			vertexData[1].position.z = position.z;
			vertexData[1].uv.x = u1;
			vertexData[1].uv.y = v0;
			vertexData[1].color = getAbgr(color);

			vertexData[2].position.x = x1;
			vertexData[2].position.y = y1;
			vertexData[2].position.z = position.z;
			vertexData[2].uv.x = u1;
			vertexData[2].uv.y = v1;
			vertexData[2].color = getAbgr(color);

			vertexData[3].position.x = x0;
			vertexData[3].position.y = y1;
			vertexData[3].position.z = position.z;
			vertexData[3].uv.x = u0;
			vertexData[3].uv.y = v1;
			vertexData[3].color = getAbgr(color);

			// Fill index buffer
			uint16_t* indices = (uint16_t*)&transientIndexBuffer.data[i*6*sizeof(uint16_t)];
			indices[0] = index + 0;
			indices[1] = index + 1;
			indices[2] = index + 2;
			indices[3] = index + 0;
			indices[4] = index + 2;
			indices[5] = index + 3;

			// Advance pen position
			xPenAdvance += glyphData.xAdvance;

			index += 4;
		}
	}

	bgfx::setVertexBuffer(&transientVertexBuffer);
	bgfx::setIndexBuffer(&transientIndexBuffer);
	bgfx::setTransform(getFloatPointer(projectionTransformMatrix));
	materialManager->createMaterial(material);
	materialManager->get(material)->bind(*resourceManager, *shaderManager, 2);
}

void Gui::drawText(const Vector2& position, uint32_t fontSize, const char* text, StringId64 font, StringId64 material, const Color4& color)
{
	drawText3D(createVector3(position.x, position.y, 0.0f), fontSize, text, font, material, color);
}

} // namespace Rio
