// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/Device.h"

#include "Core/Json/JsonR.h"
#include "Core/Strings/StringStream.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Base/Types.h"
#include "Core/Math/Vector3.h"
#include "Core/FileSystem/DiskFileSystem.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/Android/ApkFileSystem_Android.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Memory/Memory.h"
#include "Core/Base/Os.h"
#include "Core/FileSystem/Path.h"

#include "Device/OsEventQueue.h"
#include "Device/ConsoleServer.h"
#include "Device/InputDevice.h"
#include "Device/InputManager.h"
#include "Device/Log.h"
#include "Device/Profiler.h"

#include "Resource/ScriptResource.h"
#include "Resource/MaterialResource.h"
#include "Resource/BundleCompiler.h"
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

static void consoleCommandExecuteScript(void* /*data*/, ConsoleServer& /*cs*/, TcpSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	DynamicString script(ta);
	JsonRFn::parse(json, jsonObject);
	JsonRFn::parseString(jsonObject["script"], script);
	getDevice()->getScriptEnvironment()->executeScriptString(script.getCStr());
}

static void consoleCommandReloadResource(void* /*data*/, ConsoleServer& /*cs*/, TcpSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);
	StringId64 type = JsonRFn::parseResourceId(jsonObject["resourceType"]);
	StringId64 name = JsonRFn::parseResourceId(jsonObject["resourceName"]);
 	getDevice()->reload(type, name);
}

static void consoleCommandPause(void* /*data*/, ConsoleServer& /*cs*/, TcpSocket /*client*/, const char* /*json*/)
{
	getDevice()->pause();
}

static void consoleCommandUnpause(void* /*data*/, ConsoleServer& /*cs*/, TcpSocket /*client*/, const char* /*json*/)
{
	getDevice()->unpause();
}

static void consoleCommandCompileResource(void* data, ConsoleServer& cs, TcpSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	DynamicString id(ta);
	DynamicString bundleDirectory(ta);
	DynamicString platform(ta);
	JsonRFn::parseString(jsonObject["id"], id);
	JsonRFn::parseString(jsonObject["bundleDirectory"], bundleDirectory);
	JsonRFn::parseString(jsonObject["platform"], platform);

	{
		TempAllocator512 ta;
		StringStream stringStream(ta);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << id.getCStr() << "\",\"start\":true}";
		cs.send(client, StringStreamFn::getCStr(stringStream));
	}

	BundleCompiler* bc = (BundleCompiler*)data;
	bool success = bc->compile(bundleDirectory.getCStr(), platform.getCStr());
	{
		TempAllocator512 ta;
		StringStream stringStream(ta);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << id.getCStr() << "\",\"success\":" << (success ? "true" : "false") << "}";
		cs.send(client, StringStreamFn::getCStr(stringStream));
	}
}

Device::Device(const DeviceOptions& deviceOptions)
	: allocator(getDefaultAllocator(), MAX_SUBSYSTEMS_HEAP)
	, deviceOptions(deviceOptions)
	, worldList(getDefaultAllocator())
{
}

void Device::readConfig()
{
	TempAllocator4096 ta;
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
	const char* configFile = (const char*)resourceManager->get(RESOURCE_TYPE_CONFIG, configFileName);

	JsonObject config(ta);
	JsonRFn::parse(configFile, config);

	bootScriptName = JsonRFn::parseResourceId(config["bootScript"]);
	bootPackageName = JsonRFn::parseResourceId(config["bootPackage"]);

	// Platform-specific configs
	if (MapFn::has(config, FixedString(RIO_PLATFORM_NAME)))
	{
		JsonObject platform(ta);
		JsonRFn::parse(config[RIO_PLATFORM_NAME], platform);

		if (MapFn::has(platform, FixedString("windowWidth")))
		{
			configWindowWidth = (uint16_t)JsonRFn::parseInt(platform["windowWidth"]);
		}
		if (MapFn::has(platform, FixedString("windowHeight")))
		{
			configWindowHeight = (uint16_t)JsonRFn::parseInt(platform["windowHeight"]);
		}
	}

	resourceManager->unload(RESOURCE_TYPE_CONFIG, configFileName);
}

bool Device::tryProcessEvents()
{
	OsEvent event;
	bool exit = false;

	const int16_t deltaX = mouseCurrentX - mouseLastX;
	const int16_t deltaY = mouseCurrentY - mouseLastY;
	inputManager->getMouse()->setAxis(MouseAxis::CURSOR_DELTA, createVector3(deltaX, deltaY, 0.0f));
	mouseLastX = mouseCurrentX;
	mouseLastY = mouseCurrentY;

	while (getNextEvent(event))
	{
		if (event.type == OsEvent::NONE)
		{
			continue;
		}

		switch (event.type)
		{
			case OsEvent::TOUCH:
			{
				const OsTouchEvent& ev = event.touch;
				switch (ev.type)
				{
					case OsTouchEvent::POINTER:
						inputManager->getTouch()->setButtonState(ev.pointerId, ev.pressed);
						break;
					case OsTouchEvent::MOVE:
						inputManager->getTouch()->setAxis(ev.pointerId, createVector3(ev.x, ev.y, 0.0f));
						break;
					default:
						RIO_FATAL("Unknown touch event type");
						break;
				}
				break;
			}
			case OsEvent::MOUSE:
			{
				const OsMouseEvent& ev = event.mouse;
				switch (ev.type)
				{
					case OsMouseEvent::BUTTON:
						inputManager->getMouse()->setButtonState(ev.button, ev.pressed);
						break;
					case OsMouseEvent::MOVE:
						mouseCurrentX = ev.x;
						mouseCurrentY = ev.y;
						inputManager->getMouse()->setAxis(MouseAxis::CURSOR, createVector3(ev.x, ev.y, 0.0f));
						break;
					case OsMouseEvent::WHEEL:
						inputManager->getMouse()->setAxis(MouseAxis::WHEEL, createVector3(0.0f, ev.wheel, 0.0f));
						break;
					default:
						RIO_FATAL("Unknown mouse event type");
						break;
				}
				break;
			}
			case OsEvent::KEYBOARD:
			{
				const OsKeyboardEvent& ev = event.keyboard;
				inputManager->getKeyboard()->setButtonState(ev.button, ev.pressed);
				break;
			}
			case OsEvent::JOYPAD:
			{
				const OsJoypadEvent& ev = event.joypad;
				switch (ev.type)
				{
					case OsJoypadEvent::CONNECTED:
						inputManager->getJoypad(ev.index)->setIsConnected(ev.connected);
						break;
					case OsJoypadEvent::BUTTON:
						inputManager->getJoypad(ev.index)->setButtonState(ev.button, ev.pressed);
						break;
					case OsJoypadEvent::AXIS:
						inputManager->getJoypad(ev.index)->setAxis(ev.button, createVector3(ev.x, ev.y, ev.z));
						break;
					default:
						RIO_FATAL("Unknown joypad event");
						break;
				}
				break;
			}
			case OsEvent::METRICS:
			{
				const OsMetricsEvent& ev = event.metrics;
				this->width = ev.width;
				this->height = ev.height;
				bgfx::reset(ev.width, ev.height, BGFX_RESET_VSYNC);
				break;
			}
			case OsEvent::EXIT:
			{
				exit = true;
				break;
			}
			case OsEvent::PAUSE:
			{
				pause();
				break;
			}
			case OsEvent::RESUME:
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

	return exit;
}

void Device::run()
{
	consoleServer = RIO_NEW(allocator, ConsoleServer)(getDefaultAllocator());

	bool doContinue = true;

#if RIO_PLATFORM_LINUX || RIO_PLATFORM_WINDOWS
	if (deviceOptions.needToCompile || deviceOptions.isServer)
	{
		bundleCompiler = RIO_NEW(allocator, BundleCompiler)();
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_CONFIG, RESOURCE_VERSION_CONFIG, ConfigResourceFn::compile);
		
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_TEXTURE, RESOURCE_VERSION_TEXTURE, TextureResourceFn::compile);
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_MESH, RESOURCE_VERSION_MESH, MeshResourceFn::compile);
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_SHADER, RESOURCE_VERSION_SHADER, ShaderResourceFn::compile);
		
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_UNIT, RESOURCE_VERSION_UNIT, UnitResourceFn::compile);
		
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_PACKAGE, RESOURCE_VERSION_PACKAGE, PackageResourceFn::compile);
		
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_MATERIAL, RESOURCE_VERSION_MATERIAL, MaterialResourceFn::compile);
		
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_FONT, RESOURCE_VERSION_FONT, FontResourceFn::compile);
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_LEVEL, RESOURCE_VERSION_LEVEL, LevelResourceFn::compile);
		

		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE, RESOURCE_VERSION_SPRITE, SpriteResourceFn::compile);
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, SpriteAnimationResourceFn::compile);

		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_SOUND, RESOURCE_VERSION_SOUND, SoundResourceFn::compile);

		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_SCRIPT, RESOURCE_VERSION_SCRIPT, ScriptResourceFn::compile);

		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS, RESOURCE_VERSION_PHYSICS, PhysicsResourceFn::compile);
		bundleCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS_CONFIG, RESOURCE_VERSION_PHYSICS_CONFIG, PhysicsConfigResourceFn::compile);

		bundleCompiler->scan(deviceOptions.sourceDirectory);

		if (deviceOptions.isServer)
		{
			consoleServer->registerCommand(StringId32("compile"), consoleCommandCompileResource, bundleCompiler);
			consoleServer->listen(RIO_DEFAULT_COMPILER_PORT, false);

			while (true)
			{
				consoleServer->update();
				OsFn::sleep(60);
			}
		}
		else
		{
			const char* sourceDirectory = deviceOptions.sourceDirectory;
			const char* bundleDirectory = deviceOptions.bundleDirectory;
			const char* platform = deviceOptions.platformName;
			doContinue = bundleCompiler->compile(bundleDirectory, platform);
			doContinue = doContinue && deviceOptions.doContinue;
		}
	}
#endif // RIO_PLATFORM_LINUX || RIO_PLATFORM_WINDOWS

	if (doContinue == true)
	{
#if RIO_PLATFORM_ANDROID
		bundleFileSystem = RIO_NEW(allocator, ApkFileSystem)(getDefaultAllocator(), const_cast<AAssetManager*>((AAssetManager*)deviceOptions.assetManager));
#else
		const char* bundleDirectory = deviceOptions.bundleDirectory;
		if (bundleDirectory != nullptr)
		{
			char buffer[1024];
			bundleDirectory = OsFn::getCurrentWorkingDirectory(buffer, sizeof(buffer));
		}
		bundleFileSystem = RIO_NEW(allocator, DiskFileSystem)(getDefaultAllocator());
		((DiskFileSystem*)bundleFileSystem)->setPrefix(bundleDirectory);
		if (!bundleFileSystem->getDoesExist(bundleDirectory))
		{
			bundleFileSystem->createDirectory(bundleDirectory);
		}

		lastLogFile = bundleFileSystem->open(RIO_LAST_LOG, FileOpenMode::WRITE);
#endif // RIO_PLATFORM_ANDROID

		consoleServer->registerCommand(StringId32("script"), consoleCommandExecuteScript, nullptr);
		consoleServer->registerCommand(StringId32("reload"), consoleCommandReloadResource, nullptr);
		consoleServer->registerCommand(StringId32("pause"), consoleCommandPause, nullptr);
		consoleServer->registerCommand(StringId32("unpause"), consoleCommandUnpause, nullptr);
		consoleServer->listen(deviceOptions.consolePort, deviceOptions.needToWaitForConsole);

		RIO_LOGI("Initializing Rio Engine %s...", getVersion());

		ProfilerGlobalFn::init();

		resourceLoader = RIO_NEW(allocator, ResourceLoader)(*bundleFileSystem);
		resourceManager = RIO_NEW(allocator, ResourceManager)(*resourceLoader);
		
		resourceManager->registerType(RESOURCE_TYPE_TEXTURE, TextureResourceFn::load, TextureResourceFn::unload, TextureResourceFn::online, TextureResourceFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_MESH, MeshResourceFn::load, MeshResourceFn::unload, MeshResourceFn::online, MeshResourceFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_SOUND, SoundResourceFn::load, SoundResourceFn::unload, NULL,        NULL        );
		resourceManager->registerType(RESOURCE_TYPE_UNIT, UnitResourceFn::load, UnitResourceFn::unload, NULL,        NULL        );
		resourceManager->registerType(RESOURCE_TYPE_SPRITE, SpriteResourceFn::load, SpriteResourceFn::unload, SpriteResourceFn::online, SpriteResourceFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_PACKAGE, PackageResourceFn::load, PackageResourceFn::unload, NULL,        NULL        );
		
		resourceManager->registerType(RESOURCE_TYPE_MATERIAL, MaterialResourceFn::load, MaterialResourceFn::unload, MaterialResourceFn::online, MaterialResourceFn::offline);
		
		resourceManager->registerType(RESOURCE_TYPE_FONT, FontResourceFn::load, FontResourceFn::unload, NULL,        NULL        );
		resourceManager->registerType(RESOURCE_TYPE_LEVEL, LevelResourceFn::load, LevelResourceFn::unload, NULL,        NULL        );
		resourceManager->registerType(RESOURCE_TYPE_SHADER, ShaderResourceFn::load, ShaderResourceFn::unload, ShaderResourceFn::online, ShaderResourceFn::offline);
		resourceManager->registerType(RESOURCE_TYPE_SPRITE_ANIMATION, SpriteAnimationResourceFn::load, SpriteAnimationResourceFn::unload, NULL,        NULL        );
		resourceManager->registerType(RESOURCE_TYPE_CONFIG, ConfigResourceFn::load, ConfigResourceFn::unload, NULL,        NULL        );

		resourceManager->registerType(RESOURCE_TYPE_SCRIPT, ScriptResourceFn::load, ScriptResourceFn::unload, NULL, NULL);
		resourceManager->registerType(RESOURCE_TYPE_PHYSICS, PhysicsResourceFn::load, PhysicsResourceFn::unload, NULL, NULL);
		resourceManager->registerType(RESOURCE_TYPE_PHYSICS_CONFIG, PhysicsConfigResourceFn::load, PhysicsConfigResourceFn::unload, NULL, NULL);

		readConfig();

		bgfxAllocator = RIO_NEW(allocator, BgfxAllocator)(getDefaultAllocator());
		bgfxCallback = RIO_NEW(allocator, BgfxCallback)();

		mainDisplay = DisplayFn::create(allocator);
		mainWindow = WindowFn::create(allocator);
		mainWindow->open(deviceOptions.windowX
			, deviceOptions.windowY
			, configWindowWidth
			, configWindowHeight
			, deviceOptions.parentWindow
			);
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

		bootResourcePackage = createResourcePackage(bootPackageName);
		bootResourcePackage->load();
		bootResourcePackage->flush();

		scriptEnvironment->loadScriptLibraries();
		scriptEnvironment->execute((ScriptResource*)resourceManager->get(RESOURCE_TYPE_SCRIPT, bootScriptName));
		scriptEnvironment->callGlobalFunction("init", 0);

		lastTime = OsFn::getClockTime();

		RIO_LOGD("Engine initialized");

		while (tryProcessEvents() == false && quitRequested == false)
		{
			currentTime = OsFn::getClockTime();
			const int64_t time = currentTime - lastTime;
			lastTime = currentTime;
			const double frequency = (double) OsFn::getClockFrequency();
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
					RECORD_FLOAT("lua.update", (t1 - t0)*(1.0 / frequency));
				}
				{
					const int64_t t0 = OsFn::getClockTime();
					scriptEnvironment->callGlobalFunction("render", 1, ARGUMENT_FLOAT, getLastDeltaTime());
					const int64_t t1 = OsFn::getClockTime();
					RECORD_FLOAT("lua.render", (t1 - t0)*(1.0 / frequency));
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

	consoleServer->shutdown();
	RIO_DELETE(allocator, consoleServer);
	RIO_DELETE(allocator, bundleCompiler);

	allocator.clear();
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

	const float* view = getFloatPointer(world.getCameraViewMatrix(camera));
	const float* proj = getFloatPointer(world.getCameraProjectionMatrix(camera));

	bgfx::setViewTransform(0, view, proj);
	bgfx::setViewTransform(1, view, proj);
	bgfx::setViewTransform(2, getFloatPointer(MATRIX4X4_IDENTITY), getFloatPointer(MATRIX4X4_IDENTITY));
	bgfx::setViewSeq(2, true);

	bgfx::touch(0);
	bgfx::touch(1);
	bgfx::touch(2);

	world.setCameraViewportMetrics(camera, 0, 0, width, height);

	world.render(camera);
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

	RIO_ASSERT(false, "Bad world");
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
	const void* oldResource = resourceManager->get(type, name);
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

BundleCompiler* Device::getBundleCompiler()
{
	return bundleCompiler;
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