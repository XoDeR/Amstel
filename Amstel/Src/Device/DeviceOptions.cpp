// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/DeviceOptions.h"
#include "Core/Base/CommandLine.h"
#include "Device/Log.h"
#include "Core/FileSystem/Path.h"

#include <stdlib.h>

namespace Rio
{

static void help(const char* message = nullptr)
{
	RIO_LOGI(
		"Amstel Game Engine\n"
		"\n"
		"\n"
		"Usage:\n"
		"  Rio [options]\n"
		"\n"
		"Options:\n"
		"  -h --help                  Display this help.\n"
		"  -v --version               Display engine version.\n"
		"  --sourceDirectory <path>   Use <path> as the source directory for resource compilation.\n"
		"  --dataDirectory <path>     Use <path> as the destination directory for compiled resources.\n"
		"  --bootDirectory <path>     Boot the engine with the 'boot.config' from given <path>.\n"
		"  --compile                  Do a full compile of the resources.\n"
		"  --platform <platform>      Compile resources for the given <platform>.\n"
		"      linux\n"
		"      windows\n"
		"      android\n"
		"      ios\n"
		"      osx\n"
		"  --continue                 Run the engine after resource compilation.\n"
		"  --consolePort <port>       Set port of the console.\n"
		"  --waitForConsole           Wait for a console connection before starting up.\n"
		"  --parentWindow <handle>    Set the parent window <handle> of the main window.\n"
		"  --server                   Run the engine in server mode.\n"
	);

	if (message != nullptr)
	{
		RIO_LOGE("Error: %s\n", message);
	}
}

DeviceOptions::DeviceOptions(int commandLineArgumentListCount, const char** commandLineArgumentList)
	: commandLineArgumentListCount(commandLineArgumentListCount)
	, commandLineArgumentList(commandLineArgumentList)
{
}

int DeviceOptions::parse()
{
	CommandLine commandLine(commandLineArgumentListCount, commandLineArgumentList);

	if (commandLine.hasArgument("help", 'h'))
	{
		help();
		return EXIT_FAILURE;
	}

	if (commandLine.hasArgument("version", 'v'))
	{
		RIO_LOGI(RIO_VERSION);
		return EXIT_FAILURE;
	}

	sourceDirectory = commandLine.getParameter(0, "sourceDirectory");
	dataDirectory = commandLine.getParameter(0, "dataDirectory");

	mappedSourceDirectoryName = commandLine.getParameter(0, "mappedSourceDirectory");
	if (mappedSourceDirectoryName != nullptr)
	{
		mappedSourceDirectoryPrefix = commandLine.getParameter(1, "mappedSourceDirectory");
		if (mappedSourceDirectoryPrefix == nullptr)
		{
			help("Mapped source directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	needToCompile = commandLine.hasArgument("compile");
	if (needToCompile == true)
	{
		platformName = commandLine.getParameter(0, "platform");
		if (platformName == nullptr)
		{
			help("Platform must be specified.");
			return EXIT_FAILURE;
		}
		else if (true
			&& strcmp("android", platformName) != 0
			&& strcmp("linux", platformName) != 0
			&& strcmp("windows", platformName) != 0
			)
		{
			help("Unknown platform.");
			return EXIT_FAILURE;
		}

		if (sourceDirectory == nullptr)
		{
			help("Source directory must be specified.");
			return EXIT_FAILURE;
		}

		if (dataDirectory == nullptr)
		{
			help("Data directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	isServer = commandLine.hasArgument("server");
	if (isServer == true)
	{
		if (sourceDirectory == nullptr)
		{
			help("Source directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	if (dataDirectory != nullptr)
	{
		if (!PathFn::getIsAbsolute(dataDirectory))
		{
			help("Data directory must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (sourceDirectory != nullptr)
	{
		if (!PathFn::getIsAbsolute(sourceDirectory))
		{
			help("Source directory must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (mappedSourceDirectoryPrefix != nullptr)
	{
		if (!PathFn::getIsAbsolute(mappedSourceDirectoryPrefix))
		{
			help("Mapped source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	doContinue = commandLine.hasArgument("continue");

	bootDirectory = commandLine.getParameter(0, "bootDirectory");
	if (bootDirectory != nullptr)
	{
		if (!PathFn::getIsRelative(bootDirectory))
		{
			help("Boot directory must be relative.");
			return EXIT_FAILURE;
		}
	}

	needToWaitForConsole = commandLine.hasArgument("waitForConsole");

	const char* parent = commandLine.getParameter(0, "parentWindow");
	if (parent != nullptr)
	{
		if (sscanf(parent, "%u", &parentWindow) != 1)
		{
			help("Parent window is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* port = commandLine.getParameter(0, "consolePort");
	if (port != nullptr)
	{
		if (sscanf(port, "%hu", &consolePort) != 1)
		{
			help("Console port is invalid.");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka