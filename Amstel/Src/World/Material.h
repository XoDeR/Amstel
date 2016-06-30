// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

namespace Rio
{

struct Material
{
	void bind(ResourceManager& resourceManager, ShaderManager& shaderManager, uint8_t view = 0) const;
	void setFloat(StringId32 name, float value);
	void setVector2(StringId32 name, const Vector2& value);
	void setVector3(StringId32 name, const Vector3& value);

	const MaterialResource* materialResource;
	char* data;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka