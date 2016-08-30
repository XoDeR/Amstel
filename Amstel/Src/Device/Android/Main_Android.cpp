// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_ANDROID

#include "Core/Thread/Thread.h"
#include "Device/Device.h"
#include "Device/DeviceEventQueue.h"

#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>

#include <bgfx/bgfxplatform.h>

#include <jni.h>

#include <stdlib.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace Rio
{

static bool exitIsRequested = false;

struct MainThreadArgs
{
	DeviceOptions* deviceOptions;
};

int32_t mainThreadFunction(void* data)
{
	MainThreadArgs* mainThreadArgs = (MainThreadArgs*)data;
	Rio::run(*mainThreadArgs->deviceOptions);
	exitIsRequested = true;
	return EXIT_SUCCESS;
}

struct AndroidDevice
{
	void run(struct android_app* androidApp, DeviceOptions& deviceOptions)
	{
		mainThreadArgs.deviceOptions = &deviceOptions;

		androidApp->userData = this;
		androidApp->onAppCmd = Rio::AndroidDevice::onAppCommand;
		androidApp->onInputEvent = Rio::AndroidDevice::onInputEvent;
		ANativeActivity_setWindowFlags(androidApp->activity
			, AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON
			, 0
			);

		while (androidApp->destroyRequested == 0)
		{
			int32_t num;
			android_poll_source* source;
			ALooper_pollAll(-1, NULL, &num, (void**)&source);

			if (source != nullptr)
			{
				source->process(androidApp, source);
			}
		}

		mainThread.stop();
	}

	void processCommand(struct android_app* androidApp, int32_t cmd)
	{
		switch (cmd)
		{
			case APP_CMD_SAVE_STATE:
			{
				
			}
			break;
			case APP_CMD_INIT_WINDOW:
			{
				RIO_ASSERT(androidApp->window != NULL, "Android window is NULL");
				bgfx::androidSetWindow(app->window);
				// Push metrics here since Android does not trigger APP_CMD_WINDOW_RESIZED
				const int32_t width = ANativeWindow_getWidth(androidApp->window);
				const int32_t height = ANativeWindow_getHeight(androidApp->window);
				deviceEventQueue.pushResolutionEvent(width, height);
				if (!mainThread.getIsRunning())
				{
					mainThread.start(mainThreadFunction, &mainThreadArgs);
				}
			}
			break;
			case APP_CMD_TERM_WINDOW:
			{
				// The window is being hidden or closed, clean it up.
			}
			break;
			case APP_CMD_WINDOW_RESIZED:
			{
				// Not triggered by Android
			}
			break;
			case APP_CMD_GAINED_FOCUS:
			{
			}
			break;
			case APP_CMD_LOST_FOCUS:
			{
			}
			break;
			case APP_CMD_DESTROY:
			{
				deviceEventQueue.pushExitEvent();
			}
			break;
		}
	}

	int32_t processInput(struct android_app* androidApp, AInputEvent* event)
	{
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
		{
			const int32_t action = AMotionEvent_getAction(event);
			const int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			const int32_t pointerCount = AMotionEvent_getPointerCount(event);

			const int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
			const float x = AMotionEvent_getX(event, pointerIndex);
			const float y = AMotionEvent_getY(event, pointerIndex);

			const int32_t actionMasked = (action & AMOTION_EVENT_ACTION_MASK);

			switch (actionMasked)
			{
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
				{
					deviceEventQueue.pushButtonEvent(InputDeviceType::TOUCHSCREEN
						, 0
						, pointerId
						, true
						);
				}
				break;
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{
					deviceEventQueue.pushButtonEvent(InputDeviceType::TOUCHSCREEN
						, 0
						, pointerId
						, false
						);
				}
				break;
				case AMOTION_EVENT_ACTION_OUTSIDE:
				case AMOTION_EVENT_ACTION_CANCEL:
				{
					deviceEventQueue.pushButtonEvent(InputDeviceType::TOUCHSCREEN
						, 0
						, pointerId
						, false
						);
				}
				break;
				case AMOTION_EVENT_ACTION_MOVE:
				{
					for (int index = 0; index < pointerCount; index++)
					{
						const float xx = AMotionEvent_getX(event, index);
						const float yy = AMotionEvent_getY(event, index);
						const int32_t pointerId = AMotionEvent_getPointerId(event, index);
						deviceEventQueue.pushAxisEvent(InputDeviceType::TOUCHSCREEN
							, 0
							, pointerId
							, xx
							, yy
							, 0.0f
							);
					}
				}
				break;
			}

			return 1;
		}
		else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		{
			const int32_t keyCode = AKeyEvent_getKeyCode(event);
			const int32_t keyAction = AKeyEvent_getAction(event);

			if (keyCode == AKEYCODE_BACK)
			{
				deviceEventQueue.pushButtonEvent(InputDeviceType::KEYBOARD
					, 0
					, KeyboardButton::ESCAPE
					, keyaction == AKEY_EVENT_ACTION_DOWN ? true : false
					);
			}

			return 1;
		}

		return 0;
	}

	static int32_t onInputEvent(struct android_app* androidApp, AInputEvent* inputEvent)
	{
		return ((AndroidDevice*)androidApp->userData)->processInput(androidApp, inputEvent);
	}

	static void onAppCommand(struct android_app* androidApp, int32_t cmd)
	{
		((AndroidDevice*)androidApp->userData)->processCommand(androidApp, cmd);
	}

public:
	DeviceEventQueue deviceEventQueue;
	Thread mainThread;
	MainThreadArgs mainThreadArgs;
};

struct WindowAndroid : public Window
{
	WindowAndroid()
	{
	}

	void open(uint16_t /*x*/, uint16_t /*y*/, uint16_t /*width*/, uint16_t /*height*/, uint32_t /*parent*/)
	{
	}

	void close()
	{
	}

	void setupBgfx()
	{
	}

	void show()
	{
	}

	void hide()
	{
	}

	void resize(uint16_t /*width*/, uint16_t /*height*/)
	{
	}

	void move(uint16_t /*x*/, uint16_t /*y*/)
	{
	}

	void minimize()
	{
	}

	void restore()
	{
	}

	const char* getTitle()
	{
		return nullptr;
	}

	void setTitle(const char* /*title*/)
	{
	}

	void setShowCursor(bool /*show*/)
	{
	}

	void setFullscreen(bool /*isFullscreen*/)
	{
	}

	void* getHandle()
	{
		return nullptr;
	}
};

namespace WindowFn
{
	Window* create(Allocator& a)
	{
		return RIO_NEW(a, WindowAndroid)();
	}

	void destroy(Allocator& a, Window& window)
	{
		RIO_DELETE(a, &window);
	}
} // namespace WindowFn

struct DisplayAndroid : public Display
{
	void getModes(Array<DisplayMode>& /*modes*/)
	{
	}

	void setMode(uint32_t /*id*/)
	{
	}
};

namespace DisplayFn
{
	Display* create(Allocator& a)
	{
		return RIO_NEW(a, DisplayAndroid)();
	}

	void destroy(Allocator& a, Display& display)
	{
		RIO_DELETE(a, &display);
	}
} // namespace DisplayFn

static AndroidDevice androidDevice;

bool getNextEvent(OsEvent& ev)
{
	return androidDevice.deviceEventQueue.popEvent(ev);
}

} // namespace Rio

void android_main(struct android_app* app)
{
	using namespace Rio;

	// Make sure glue isn't stripped
	app_dummy();

	MemoryGlobalFn::init();

	DeviceOptions deviceOptions(0, NULL);
	deviceOptions.assetManager = app->activity->assetManager;

	Rio::androidDevice.run(app, deviceOptions);
	MemoryGlobalFn::shutdown();
}

#endif // RIO_PLATFORM_ANDROID
// Copyright (c) 2016 Volodymyr Syvochka