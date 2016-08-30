// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/Device.h"

#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Strings/StringStream.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Base/Types.h"
#include "Core/Base/Os.h"
#include "Core/Math/Vector3.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/Path.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/FileSystemDisk.h"
#if RIO_PLATFORM_ANDROID
#include "Core/FileSystem/Android/FileSystemApk_Android.h"
#endif //RIO_PLATFORM_ANDROID
#include "Core/Memory/Memory.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/Math/Matrix4x4.h"

#include "Device/DeviceEventQueue.h"
#include "Device/ConsoleServer.h"
#include "Device/ConsoleApi.h"
#include "Device/InputDevice.h"
#include "Device/InputManager.h"
#include "Device/Log.h"
#include "Device/Profiler.h"

#include "Resource/ScriptResource.h"
#include "Resource/MaterialResource.h"
#include "Resource/DataCompiler.h"
#include "Resource/ConfigResource.h"
#include "Resource/LevelResource.h"
#include "Resource/PackageResource.h"
#include "Resource/FontResource.h"
#include "Resource/MeshResource.h"
#include "Resource/SoundResource.h"
#include "Resource/SpriteResource.h"
#include "Resource/ShaderResource.h"
#include "Resource/PhysicsResource.h"
#include "Resource/TextureResource.h"
#include "Resource/ResourceLoader.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourcePackage.h"
#include "Resource/UnitResource.h"

#include "World/ShaderManager.h"
#include "World/World.h"
#include "World/UnitManager.h"
#include "World/MaterialManager.h"
#include "World/Audio.h"
#include "World/Physics.h"

#include "Script/ScriptEnvironment.h"

#include <bgfx/bgfx.h>
#include <bx/allocator.h>

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace Rio
{

extern bool getNextEvent(OsEvent& ev);

struct BgfxCallback : public bgfx::CallbackI
{
	virtual void fatal(bgfx::Fatal::Enum code, const char* str)
	{
		RIO_ASSERT(false, "Fatal error: 0x%08x: %s", code, str);
		RIO_UNUSED(code);
		RIO_UNUSED(str);
	}

	virtual void traceVargs(const char* /*filePath*/, uint16_t /*line*/, const char* format, va_list argList)
	{
		char buffer[2048];
		strncpy(buffer, format, sizeof(buffer));
		buffer[getStringLength32(buffer)-1] = '\0'; // Remove trailing newline
		RIO_LOGDV(buffer, argList);
	}

	virtual uint32_t cacheReadSize(uint64_t /*id*/)
	{
		return 0;
	}

	virtual bool cacheRead(uint64_t /*id*/, void* /*data*/, uint32_t /*size*/)
	{
		return false;
	}

	virtual void cacheWrite(uint64_t /*id*/, const void* /*data*/, uint32_t /*size*/)
	{
	}

	virtual void screenShot(const char* /*filePath*/, uint32_t /*width*/, uint32_t /*height*/, uint32_t /*pitch*/, const void* /*data*/, uint32_t /*size*/, bool /*yFlip*/)
	{
	}

	virtual void captureBegin(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*pitch*/, bgfx::TextureFormat::Enum /*format*/, bool /*yFlip*/)
	{
	}

	virtual void captureEnd()
	{
	}

	virtual void captureFrame(const void* /*data*/, uint32_t /*size*/)
	{
	}
};

struct BgfxAllocator : public bx::AllocatorI
{
	BgfxAllocator(Allocator& a)
		: allocator(a, "bgfx")
	{
	}

	virtual void* realloc(void* ptr, size_t size, size_t align, const char* /*file*/, uint32_t /*line*/)
	{
		if (!ptr)
		{
			return allocator.allocate((uint32_t)size, (uint32_t)align == 0 ? 1 : (uint32_t)align);
		}

		if (size == 0)
		{
			allocator.deallocate(ptr);
			return nullptr;
		}

		// Realloc
		void* p = allocator.allocate((uint32_t)size, (uint32_t)align == 0 ? 1 : (uint32_t)align);
		allocator.deallocate(ptr);
		return p;
	}
private:
	ProxyAllocator allocator;
};

Device::Device(const DeviceOptions& deviceOptions)
	: allocator(getDefaultAllocator(), MAX_SUBSYSTEMS_HEAP)
	, deviceOptions(deviceOptions)
	, bootConfig(getDefaultAllocator())
	, worldList(getDefaultAllocator())
{
}

void Device::readConfig()
{
	TempAllocator512 ta;
	DynamicString bootDirectory(ta);

	if (deviceOptions.bootDirectory != nullptr)
	{
		bootDirectory += deviceOptions.bootDirectory;
		bootDirectory += '/';
	}

	bootDirectory += RIO_BOOT_CONFIG;

	const StringId64 configFileName(bootDirectory.getCStr());

	resourceManager->load(RESOURCE_TYPE_CONFIG, configFileName);
	resourceManager->flush();
	bootConfig.parse((const char*)resourceManager->get(RESOURCE_TYPE_CONFIG, configFileName));
	resourceManager->unload(RESOURCE_TYPE_CONFIG, configFileName);
}

bool Device::processEvents(int16_t& mouseX, int16_t& mouseY, int16_t& mouseLastX, int16_t& mouseLastY, bool isVsyncEnabled)
{
	bool exit = false;
	bool reset = false;

	OsEvent event;
	while (getNextEvent(event))
	{
		if (event.type == OsEventType::NONE)
		{
			continue;
		}

		switch (event.type)
		{
		case OsEventType::BUTTON:
		{
			const ButtonEvent ev = event.button;
			switch (ev.deviceId)
			{
			case InputDeviceType::KEYBOARD:
				inputManager->getKeyboard()->setButtonState(ev.buttonIndex, ev.pressed);
				break;

			case InputDeviceType::MOUSE:
				inputManager->getMouse()->setButtonState(ev.buttonIndex, ev.pressed);
				break;

			case InputDeviceType::TOUCHSCREEN:
				inputManager->getTouch()->setButtonState(ev.buttonIndex, ev.pressed);
				break;

			case InputDeviceType::JOYPAD:
				inputManager->getJoypad(ev.deviceIndex)->setButtonState(ev.buttonIndex, ev.pressed);
				break;
			}
		}
		break;

		case OsEventType::AXIS:
		{
			const AxisEvent ev = event.axis;
			switch (ev.deviceId)
			{
			case InputDeviceType::MOUSE:
				inputManager->getMouse()->setAxis(ev.axisIndex, createVector3(ev.axisX, ev.axisY, ev.axisZ));
				if (ev.axisIndex == MouseAxis::CURSOR)
				{
					mouseX = ev.axisX;
					mouseY = ev.axisY;
				}
				break;

			case InputDeviceType::JOYPAD:
				inputManager->getJoypad(ev.deviceIndex)->setAxis(ev.axisIndex, createVector3(ev.axisX, ev.axisY, ev.axisZ));
				break;
			}
		}
		break;

		case OsEventType::STATUS:
		{
			const StatusEvent ev = event.status;
			switch (ev.deviceId)
			{
			case InputDeviceType::JOYPAD:
				inputManager->getJoypad(ev.deviceIndex)->setIsConnected(ev.connected);
				break;
			}
		}
		break;

		case OsEventType::RESOLUTION:
		{
			const ResolutionEvent& ev = event.resolution;
			this->width = ev.width;
			this->height = ev.height;
			reset = true;
		}
		break;
		case OsEventType::EXIT:
		{
			exit = true;
			break;
		}
		case OsEventType::PAUSE:
		{
			pause();
			break;
		}
		case OsEventType::RESUME:
		{
			unpause();
			break;
		}
		default:
		{
			RIO_FATAL("Unknown Os Event");
			break;
		}
		}
	}

	const int16_t deltaX = mouseCurrentX - mouseLastX;
	const int16_t deltaY = mouseCurrentY - mouseLastY;
	inputManager->getMouse()->setAxis(MouseAxis::CURSOR_DELTA, createVector3(deltaX, deltaY, 0.0f));
	mouseLastX = mouseCurrentX;
	mouseLastY = mouseCurrentY;

	if (reset)
	{
		bgfx::reset(this->width, this->height, (isVsyncEnabled ? BGFX_RESET_VSYNC : BGFX_RESET_NONE));
	}

	return exit;
}

void Device::run()
{
	consoleServer = RIO_NEW(allocator, ConsoleServer)(getDefaultAllocator());
	loadConsoleApi(*consoleServer);

	bool doContinue = true;

#if RIO_PLATFORM_LINUX || RIO_PLATFORM_WINDOWS
	if (deviceOptions.needToCompile || deviceOptions.isServer)
	{
		dataCompiler = RIO_NEW(allocator, DataCompiler)();
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_CONFIG, RESOURCE_VERSION_CONFIG, ConfigResourceInternalFn::compile);
		
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_TEXTURE, RESOURCE_VERSION_TEXTURE, TextureResourceInternalFn::compile);
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_MESH, RESOURCE_VERSION_MESH, MeshResourceInternalFn::compile);
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SHADER, RESOURCE_VERSION_SHADER, ShaderResourceInternalFn::compile);
		
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_UNIT, RESOURCE_VERSION_UNIT, UnitResourceInternalFn::compile);
		
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PACKAGE, RESOURCE_VERSION_PACKAGE, PackageResourceInternalFn::compile);
		
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_MATERIAL, RESOURCE_VERSION_MATERIAL, MaterialResourceInternalFn::compile);
		
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_FONT, RESOURCE_VERSION_FONT, FontResourceInternalFn::compile);
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_LEVEL, RESOURCE_VERSION_LEVEL, LevelResourceInternalFn::compile);
		

		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE, RESOURCE_VERSION_SPRITE, SpriteResourceInternalFn::compile);
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, SpriteAnimationResourceInternalFn::compile);

		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SOUND, RESOURCE_VERSION_SOUND, SoundResourceInternalFn::compile);

		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SCRIPT, RESOURCE_VERSION_SCRIPT, ScriptResourceInternalFn::compile);

		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS, RESOURCE_VERSION_PHYSICS, PhysicsResourceInternalFn::compile);
		dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS_CONFIG, RESOURCE_VERSION_PHYSICS_CONFIG, PhysicsConfigResourceInternalFn::compile);

		dataCompiler->mapSourceDirectory("", deviceOptions.sourceDirectory);

		if (deviceOptions.mappedSourceDirectoryName)
		{
			dataCompiler->mapSourceDirectory(deviceOptions.mappedSourceDirectoryName
				, deviceOptions.mappedSourceDirectoryPrefix
				);
		}
		dataCompiler->scan();

		if (deviceOptions.isServer == true)
		{
			consoleServer->listen(RIO_DEFAULT_COMPILER_PORT, false);

			while (true)
			{
				consoleServer->update();
				OsFn::sleep(60);
			}
		}
		else
		{
			const char* dataDirectory = deviceOptions.dataDirectory;
			const char* platform = deviceOptions.platformName;
			doContinue = dataCompiler->compile(dataDirectory, platform);
			doContinue = doContinue && deviceOptions.doContinue;
		}
	}
#endif // RIO_PLATFORM_LINUX || RIO_PLATFORM_WINDOWS

	if (doContinue == true)
	{
		consoleServer->listen(deviceOptions.consolePort, deviceOptions.needToWaitForConsole);
#if RIO_PLATFORM_ANDROID
		bundleFileSystem = RIO_NEW(allocator, FileSystemApk)(getDefaultAllocator(), const_cast<AAssetManager*>((AAssetManager*)deviceOptions.assetManager));
#else
		const char* dataDirectory = deviceOptions.dataDirectory;
		if (dataDirectory != nullptr)
		{
			char buffer[1024];
			dataDirectory = OsFn::getCurrentWorkingDirectory(buffer, sizeof(buffer));
		}
		bundleFileSystem = RIO_NEW(allocator, FileSystemDisk)(getDefaultAllocator());
		static_cast<FileSystemDisk*>(bundleFileSystem)->setPrefix(dataDirectory);
		if (!bundleFileSystem->getDoesExist(dataDirectory))
		{
			bundleFileSystem->createDirectory(dataDirectory);
		}

		lastLogFile = bundleFileSystem->open(RIO_LAST_LOG, FileOpenMode::WRITE);
#endif // RIO_PLATFORM_ANDROID
		RIO_LOGI("Initializing Rio Engine %s...", getVersion());

		ProfilerGlobalFn::init();

		resourceLoader = RIO_NEW(allocator, ResourceLoader)(*bundleFileSystem);
		resourceManager = RIO_NEW(allocator, ResourceManager)(*resourceLoader);
		
		resourceManager->registerType(RESOURCE_TYPE_TEXTURE, TextureResourceInternalFn::load, TextureResourceInternalFn::unload, TextureResourceInternalFn::online, TextureResourceInternalFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_MESH, MeshResourceInternalFn::load, MeshResourceInternalFn::unload, MeshResourceInternalFn::online, MeshResourceInternalFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_SOUND, SoundResourceInternalFn::load, SoundResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_UNIT, UnitResourceInternalFn::load, UnitResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_SPRITE, SpriteResourceInternalFn::load, SpriteResourceInternalFn::unload, SpriteResourceInternalFn::online, SpriteResourceInternalFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_PACKAGE, PackageResourceInternalFn::load, PackageResourceInternalFn::unload, nullptr, nullptr);
		
		resourceManager->registerType(RESOURCE_TYPE_MATERIAL, MaterialResourceInternalFn::load, MaterialResourceInternalFn::unload, MaterialResourceInternalFn::online, MaterialResourceInternalFn::offline);
		
		resourceManager->registerType(RESOURCE_TYPE_FONT, FontResourceInternalFn::load, FontResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_LEVEL, LevelResourceInternalFn::load, LevelResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_SHADER, ShaderResourceInternalFn::load, ShaderResourceInternalFn::unload, ShaderResourceInternalFn::online, ShaderResourceInternalFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_SPRITE_ANIMATION, SpriteAnimationResourceInternalFn::load, SpriteAnimationResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_CONFIG, ConfigResourceInternalFn::load, ConfigResourceInternalFn::unload, nullptr, nullptr);

		resourceManager->registerType(RESOURCE_TYPE_SCRIPT, ScriptResourceInternalFn::load, ScriptResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_PHYSICS, PhysicsResourceInternalFn::load, PhysicsResourceInternalFn::unload, nullptr, nullptr);
		resourceManager->registerType(RESOURCE_TYPE_PHYSICS_CONFIG, PhysicsConfigResourceInternalFn::load, PhysicsConfigResourceInternalFn::unload, nullptr, nullptr);

		readConfig();

		bgfxAllocator = RIO_NEW(allocator, BgfxAllocator)(getDefaultAllocator());
		bgfxCallback = RIO_NEW(allocator, BgfxCallback)();

		mainDisplay = DisplayFn::create(allocator);
		mainWindow = WindowFn::create(allocator);
		mainWindow->open(deviceOptions.windowX
			, deviceOptions.windowY
			, bootConfig.windowWidth
			, bootConfig.windowHeight
			, deviceOptions.parentWindow
			);
		mainWindow->setTitle(bootConfig.windowTitle.getCStr());
		mainWindow->setFullscreen(bootConfig.isFullscreen);
		mainWindow->setupBgfx();

		bgfx::init(bgfx::RendererType::Count
			, BGFX_PCI_ID_NONE
			, 0
			, bgfxCallback
			, bgfxAllocator
			);

		shaderManager = RIO_NEW(allocator, ShaderManager)(getDefaultAllocator());
		materialManager = RIO_NEW(allocator, MaterialManager)(getDefaultAllocator(), *resourceManager);
		inputManager = RIO_NEW(allocator, InputManager)(getDefaultAllocator());
		unitManager = RIO_NEW(allocator, UnitManager)(getDefaultAllocator());
		scriptEnvironment = RIO_NEW(allocator, ScriptEnvironment)();

		AudioGlobalFn::init();

#if RIO_PHYSICS_NULL == 0
		PhysicsGlobalFn::init(allocator);
#endif // RIO_PHYSICS_NULL

		ResourcePackage* bootResourcePackage = createResourcePackage(bootConfig.bootPackageName);
		bootResourcePackage->load();
		bootResourcePackage->flush();

		scriptEnvironment->loadScriptLibraries();
		scriptEnvironment->execute((ScriptResource*)resourceManager->get(RESOURCE_TYPE_SCRIPT, bootConfig.bootScriptName));
		scriptEnvironment->callGlobalFunction("init", 0);

		RIO_LOGD("Engine initialized");

		int16_t mouse_x = 0;
		int16_t mouse_y = 0;
		int16_t mouse_last_x = 0;
		int16_t mouse_last_y = 0;

		int64_t lastTime = OsFn::getClockTime();
		int64_t currentTime = 0;

		while (processEvents(mouse_x, mouse_y, mouse_last_x, mouse_last_y, bootConfig.vSync) == false && quitRequested == false)
		{
			currentTime = OsFn::getClockTime();
			const int64_t time = currentTime - lastTime;
			lastTime = currentTime;
			const double frequency = static_cast<double>(OsFn::getClockFrequency());
			lastDeltaTime = float(time * (1.0 / frequency));
			timeSinceStart += lastDeltaTime;

			ProfilerGlobalFn::clear();
			consoleServer->update();

			RECORD_FLOAT("device.dt", lastDeltaTime);
			RECORD_FLOAT("device.fps", 1.0f/ lastDeltaTime);

			if (paused == false)
			{
				resourceManager->completeRequests();

				{
					const int64_t t0 = OsFn::getClockTime();
					scriptEnvironment->callGlobalFunction("update", 1, ARGUMENT_FLOAT, getLastDeltaTime());
					const int64_t t1 = OsFn::getClockTime();
					RECORD_FLOAT("lua.update", static_cast<float>((t1 - t0)*(1.0 / frequency)));
				}
				{
					const int64_t t0 = OsFn::getClockTime();
					scriptEnvironment->callGlobalFunction("render", 1, ARGUMENT_FLOAT, getLastDeltaTime());
					const int64_t t1 = OsFn::getClockTime();
					RECORD_FLOAT("lua.render", static_cast<float>((t1 - t0)*(1.0 / frequency)));
				}
			}

			inputManager->update();

			const bgfx::Stats* stats = bgfx::getStats();
			RECORD_FLOAT("bgfx.gpu_time", float(double(stats->gpuTimeEnd - stats->gpuTimeBegin)*1000.0/stats->gpuTimerFreq));
			RECORD_FLOAT("bgfx.cpu_time", float(double(stats->cpuTimeEnd - stats->cpuTimeBegin)*1000.0/stats->cpuTimerFreq));

			bgfx::frame();
			ProfilerGlobalFn::flush();

			scriptEnvironment->resetTemporaryTypes();

			frameCount++;
		}

		scriptEnvironment->callGlobalFunction("shutdown", 0);

		bootResourcePackage->unload();
		destroyResourcePackage(*bootResourcePackage);

#if RIO_PHYSICS_NULL == 0
		PhysicsGlobalFn::shutdown(allocator);
#endif // RIO_PHYSICS_NULL
		
		AudioGlobalFn::shutdown();

		RIO_DELETE(allocator, scriptEnvironment);
		RIO_DELETE(allocator, unitManager);
		RIO_DELETE(allocator, inputManager);
		RIO_DELETE(allocator, materialManager);
		RIO_DELETE(allocator, shaderManager);
		RIO_DELETE(allocator, resourceManager);
		RIO_DELETE(allocator, resourceLoader);

		bgfx::shutdown();
		mainWindow->close();
		WindowFn::destroy(allocator, *mainWindow);
		DisplayFn::destroy(allocator, *mainDisplay);
		RIO_DELETE(allocator, bgfxCallback);
		RIO_DELETE(allocator, bgfxAllocator);

		if (lastLogFile != nullptr)
		{
			bundleFileSystem->close(*lastLogFile);
		}

		RIO_DELETE(allocator, bundleFileSystem);

		ProfilerGlobalFn::shutdown();
	}

	RIO_DELETE(allocator, dataCompiler);
	consoleServer->shutdown();
	RIO_DELETE(allocator, consoleServer);
	allocator.clear();
}

inline int Device::getCommandLineArgumentListCount() const
{
	return deviceOptions.commandLineArgumentListCount;
}

inline const char** Device::getCommandLineArgumentList() const
{
	return (const char**)deviceOptions.commandLineArgumentList;
}

inline const char* Device::getPlatform() const
{
	return RIO_PLATFORM_NAME;
}

inline const char* Device::getArchitecture() const
{
	return RIO_ARCH_NAME;
}

inline const char* Device::getVersion() const
{
	return RIO_VERSION;
}

void Device::quit()
{
	quitRequested = true;
}

void Device::pause()
{
	paused = true;
	RIO_LOGI("Engine paused.");
}

void Device::unpause()
{
	paused = false;
	RIO_LOGI("Engine unpaused.");
}

void Device::getResolution(uint16_t& width, uint16_t& height)
{
	width = this->width;
	height = this->height;
}

uint64_t Device::getFrameCount() const
{
	return frameCount;
}

float Device::getLastDeltaTime() const
{
	return lastDeltaTime;
}

double Device::getTimeSinceStart() const
{
	return timeSinceStart;
}

void Device::render(World& world, CameraInstance camera)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x353839FF
		, 1.0f
		, 0
		);

	bgfx::setViewRect(0, 0, 0, width, height);
	bgfx::setViewRect(1, 0, 0, width, height);
	bgfx::setViewRect(2, 0, 0, width, height);

	const Matrix4x4 view = world.cameraGetViewMatrix(camera);
	const Matrix4x4 proj = world.cameraGetProjectionMatrix(camera);

	bgfx::setViewTransform(0, getFloatPointer(view), getFloatPointer(proj));
	bgfx::setViewTransform(1, getFloatPointer(view), getFloatPointer(proj));
	bgfx::setViewTransform(2, getFloatPointer(MATRIX4X4_IDENTITY), getFloatPointer(MATRIX4X4_IDENTITY));
	bgfx::setViewSeq(2, true);

	bgfx::touch(0);
	bgfx::touch(1);
	bgfx::touch(2);

	float aspectRatio = (bootConfig.aspectRatio == -1.0f
		? (float)this->width / (float)this->height
		: bootConfig.aspectRatio
		);
	world.cameraSetAspect(camera, aspectRatio);
	world.cameraSetViewportMetrics(camera, 0, 0, width, height);

	world.render(view, proj);
}

World* Device::createWorld()
{
	World* world = RIO_NEW(getDefaultAllocator(), World)(getDefaultAllocator()
		, *resourceManager
		, *shaderManager
		, *materialManager
		, *unitManager
		, *scriptEnvironment
		);
	ArrayFn::pushBack(worldList, world);
	return world;
}

void Device::destroyWorld(World& world)
{
	for (uint32_t i = 0, n = ArrayFn::getCount(worldList); i < n; ++i)
	{
		if (&world == worldList[i])
		{
			RIO_DELETE(getDefaultAllocator(), &world);
			worldList[i] = worldList[n - 1];
			ArrayFn::popBack(worldList);
			return;
		}
	}

	RIO_FATAL("Bad world");
}

ResourcePackage* Device::createResourcePackage(StringId64 id)
{
	return RIO_NEW(getDefaultAllocator(), ResourcePackage)(id, *resourceManager);
}

void Device::destroyResourcePackage(ResourcePackage& rp)
{
	RIO_DELETE(getDefaultAllocator(), &rp);
}

void Device::reload(StringId64 type, StringId64 name)
{
	resourceManager->reload(type, name);
	const void* newResource = resourceManager->get(type, name);

	if (type == RESOURCE_TYPE_SCRIPT)
	{
		scriptEnvironment->execute((const ScriptResource*)newResource);
	}
}

static StringStream& sanitize(StringStream& stringStream, const char* msg)
{
	using namespace StringStreamFn;
	const char* ch = msg;
	for (; *ch; ch++)
	{
		if (*ch == '"' || *ch == '\\')
		{
			stringStream << "\\";
		}
		stringStream << *ch;
	}

	return stringStream;
}

static const char* logSeverityNameMap[] = 
{ 
	"info", 
	"warning", 
	"error", 
	"debug" 
};
RIO_STATIC_ASSERT(RIO_COUNTOF(logSeverityNameMap) == LogSeverity::COUNT);

void Device::log(const char* msg, LogSeverity::Enum severity)
{
	if (lastLogFile != nullptr)
	{
		lastLogFile->write(msg, getStringLength32(msg));
		lastLogFile->write("\n", 1);
		lastLogFile->flush();
	}

	if (consoleServer != nullptr)
	{
		TempAllocator4096 ta;
		StringStream json(ta);

		json << "{\"type\":\"message\",";
		json << "\"severity\":\"" << logSeverityNameMap[severity] << "\",";
		json << "\"message\":\"";
		sanitize(json, msg) << "\"}";

		consoleServer->send(StringStreamFn::getCStr(json));
	}
}

ConsoleServer* Device::getConsoleServer()
{
	return consoleServer;
}

DataCompiler* Device::getDataCompiler()
{
	return dataCompiler;
}

ResourceManager* Device::getResourceManager()
{
	return resourceManager;
}

ScriptEnvironment* Device::getScriptEnvironment()
{
	return scriptEnvironment;
}

InputManager* Device::getInputManager()
{
	return inputManager;
}

ShaderManager* Device::getShaderManager()
{
	return shaderManager;
}

MaterialManager* Device::getMaterialManager()
{
	return materialManager;
}

UnitManager* Device::getUnitManager()
{
	return unitManager;
}

Display& Device::getMainDisplay()
{
	return *mainDisplay;
}

Window* Device::getMainWindow()
{
	return mainWindow;
}

char buffer[sizeof(Device)];
Device* device = nullptr;

void run(const DeviceOptions& deviceOptions)
{
	RIO_ASSERT(device == nullptr, "Rio engine already initialized");
	device = new (buffer) Device(deviceOptions);
	device->run();
	device->~Device();
	device = nullptr;
}

Device* getDevice()
{
	return Rio::device;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka