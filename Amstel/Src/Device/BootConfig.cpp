#include "Device/BootConfig.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/JsonR.h"
#include "Core/Containers/Map.h"
#include "Core/Base/Platform.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

BootConfig::BootConfig(Allocator& a)
	: windowTitle(a)
{
}

bool BootConfig::parse(const char* json)
{
	TempAllocator4096 ta;
	JsonObject cfg(ta);
	JsonRFn::parse(json, cfg);

	// General configs
	bootScriptName = JsonRFn::parseResourceId(cfg["bootScript"]);
	bootPackageName = JsonRFn::parseResourceId(cfg["bootPackage"]);

	if (JsonObjectFn::has(cfg, "windowTitle"))
	{
		JsonRFn::parseString(cfg["windowTitle"], windowTitle);
	}

	// Platform-specific configs
	if (JsonObjectFn::has(cfg, RIO_PLATFORM_NAME))
	{
		JsonObject platform(ta);
		JsonRFn::parse(cfg[RIO_PLATFORM_NAME], platform);

		if (JsonObjectFn::has(platform, "renderer"))
		{
			JsonObject renderer(ta);
			JsonRFn::parse(platform["renderer"], renderer);

			if (JsonObjectFn::has(renderer, "resolution"))
			{
				JsonArray resolution(ta);
				JsonRFn::parseArray(renderer["resolution"], resolution);
				windowWidth = JsonRFn::parseInt(resolution[0]);
				windowHeight = JsonRFn::parseInt(resolution[1]);
			}
			if (JsonObjectFn::has(renderer, "aspectRatio"))
			{
				aspectRatio = JsonRFn::parseFloat(renderer["aspectRatio"]);
			}
			if (JsonObjectFn::has(renderer, "vSync"))
			{
				vSync = JsonRFn::parseBool(renderer["vSync"]);
			}
			if (JsonObjectFn::has(renderer, "isFullscreen"))
			{
				isFullscreen = JsonRFn::parseBool(renderer["isFullscreen"]);
			}
		}
	}

	return true;
}

} // namespace Rio
