// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"
#include "Core/Base/Types.h"
#include "Core/Base/Platform.h"

namespace Rio
{

struct DeviceOptions
{
	DeviceOptions(int argumentListCount, const char** argumentList);

	// Parses the command line and returns EXIT_SUCCESS if no error is found
	int parse();

	int commandLineArgumentListCount;
	const char** commandLineArgumentList;
	const char* sourceDirectory = nullptr;
	const char* bundleDirectory = nullptr;
	const char* bootDirectory = nullptr;
	const char* platformName = nullptr;
	bool needToWaitForConsole = false;
	bool needToCompile = false;
	bool doContinue = false;
	bool isServer = false;
	uint32_t parentWindow = 0;
	uint16_t consolePort = RIO_DEFAULT_CONSOLE_PORT;
	uint16_t windowX = 0;
	uint16_t windowY = 0;
	uint16_t windowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t windowHeight = RIO_DEFAULT_WINDOW_HEIGHT;
#if RIO_PLATFORM_ANDROID
	void* assetManager;
#endif // RIO_PLATFORM_ANDROID
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka