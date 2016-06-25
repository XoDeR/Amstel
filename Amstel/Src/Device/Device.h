// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"

#include "Device/ConsoleServer.h"
#include "Device/Log.h"
#include "Device/DeviceOptions.h"
#include "Device/Window.h"
#include "Device/Display.h"
#include "Device/InputTypes.h"

#include "Script/ScriptTypes.h"

#include "Resource/ResourceTypes.h"
#include "Resource/PhysicsResource.h"
#include "Resource/CompilerTypes.h"

#include "World/WorldTypes.h"

namespace Rio
{

struct BgfxAllocator;
struct BgfxCallback;

// This is the place where to look for accessing all of the engine subsystems
class Device
{
public:
	explicit Device(const DeviceOptions& opts);
	void run();

	// Returns the number of command line parameters
	int getCommandLineArgumentListCount() const 
	{ 
		return deviceOptions.commandLineArgumentListCount;
	}

	// Returns command line parameters
	const char** getCommandLineArgumentList() const 
	{ 
		return (const char**)deviceOptions.commandLineArgumentList;
	}

	// Returns a string identifying what platform the engine is running on
	const char* getPlatform() const 
	{ 
		return RIO_PLATFORM_NAME; 
	}

	// Returns a string identifying what architecture the engine is running on
	const char* getArchitecture() const 
	{ 
		return RIO_ARCH_NAME; 
	}

	// Returns a string identifying the engine version
	const char* getVersion() const 
	{ 
		return RIO_VERSION; 
	}

	// Return the number of frames rendered
	uint64_t getFrameCount() const;

	// Returns the time in seconds needed to render the last frame
	float getLastDeltaTime() const;

	// Returns the time in seconds since the the application started
	double getTimeSinceStart() const;

	// Quits the application
	void quit();
	// Pauses the engine
	void pause();
	// Unpauses the engine
	void unpause();
	// Returns the main window resolution
	void getResolution(uint16_t& width, uint16_t& height);
	// Renders <world> using <camera>
	void render(World& world, CameraInstance camera);
	// Creates a new world
	World* createWorld();
	// Destroys the world <w>
	void destroyWorld(World& w);
	// Returns the resource package <id>
	ResourcePackage* createResourcePackage(StringId64 id);
	// Destroys the resource package
	// Resources are not automatically unloaded
	// You have to call ResourcePackage::unload() before destroying a package
	void destroyResourcePackage(ResourcePackage& rp);
	// Reloads the resource
	void reload(StringId64 type, StringId64 name);
	// Logs <message> to log file and console
	void log(const char* message, LogSeverity::Enum severity);

	ConsoleServer* getConsoleServer();
	BundleCompiler* getBundleCompiler();
	ResourceManager* getResourceManager();
	ScriptEnvironment* getScriptEnvironment();
	InputManager* getInputManager();
	ShaderManager* getShaderManager();
	MaterialManager* getMaterialManager();
	UnitManager* getUnitManager();

	Display& getMainDisplay();
	Window* getMainWindow();
private:
	// Disable copying
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
private:
	void readConfig();
	bool tryProcessEvents();

	LinearAllocator allocator;
	const DeviceOptions& deviceOptions;

	ConsoleServer* consoleServer = nullptr;
	BundleCompiler* bundleCompiler = nullptr;
	FileSystem* bundleFileSystem = nullptr;
	File* lastLogFile = nullptr;
	ResourceLoader* resourceLoader = nullptr;
	ResourceManager* resourceManager = nullptr;
	BgfxAllocator* bgfxAllocator = nullptr;
	BgfxCallback* bgfxCallback = nullptr;
	ShaderManager* shaderManager = nullptr;
	MaterialManager* materialManager = nullptr;
	InputManager* inputManager = nullptr;
	UnitManager* unitManager = nullptr;
	ScriptEnvironment* scriptEnvironment = nullptr;
	Display* mainDisplay = nullptr;
	Window* mainWindow = nullptr;

	StringId64 bootPackageName;
	StringId64 bootScriptName;
	ResourcePackage* bootResourcePackage = nullptr;

	uint16_t configWindowX = 0;
	uint16_t configWindowY = 0;
	uint16_t configWindowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t configWindowHeight = RIO_DEFAULT_WINDOW_HEIGHT;

	Array<World*> worldList;

	uint16_t width = 0;
	uint16_t height = 0;
	int16_t mouseCurrentX = 0;
	int16_t mouseCurrentY = 0;
	int16_t mouseLastX = 0;
	int16_t mouseLastY = 0;

	bool quitRequested = false;
	bool paused = false;

	uint64_t frameCount = 0;
	int64_t lastTime = 0;
	int64_t currentTime = 0;
	float lastDeltaTime = 0.0f;
	double timeSinceStart = 0.0;
};

// Runs the engine
void run(const DeviceOptions& opts);

// Returns the device
Device* getDevice();

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
