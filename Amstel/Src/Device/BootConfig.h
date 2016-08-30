// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"

namespace Rio
{

struct BootConfig
{
	BootConfig(Allocator& a);
	bool parse(const char* json);

	StringId64 bootScriptName;
	StringId64 bootPackageName;
	DynamicString windowTitle;
	uint16_t windowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t windowHeight = RIO_DEFAULT_WINDOW_HEIGHT;
	float aspectRatio = -1.0f;
	bool vSync = true;
	bool isFullscreen = false;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka