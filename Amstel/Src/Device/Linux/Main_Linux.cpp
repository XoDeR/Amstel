// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_LINUX

#include "Core/Base/Os.h"
#include "Core/Containers/Array.h"
#include "Core/Base/CommandLine.h"
#include "Core/Thread/Thread.h"
#include "Core/UnitTests.cpp"
#include "Device/Device.h"
#include "Device/OsEventQueue.h"
#include "Device/Window.h"
#include "Resource/PhysicsResource.h"

#include <bgfx/bgfxplatform.h>
#include <stdlib.h>
#include <string.h> // memset
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace Rio
{

static KeyboardButton::Enum getKeyboardButtonFromX11Key(KeySym x11Key)
{
	switch (x11Key)
	{
		case XK_BackSpace:    return KeyboardButton::BACKSPACE;
		case XK_Tab:          return KeyboardButton::TAB;
		case XK_space:        return KeyboardButton::SPACE;
		case XK_Escape:       return KeyboardButton::ESCAPE;
		case XK_Return:       return KeyboardButton::ENTER;
		case XK_F1:           return KeyboardButton::F1;
		case XK_F2:           return KeyboardButton::F2;
		case XK_F3:           return KeyboardButton::F3;
		case XK_F4:           return KeyboardButton::F4;
		case XK_F5:           return KeyboardButton::F5;
		case XK_F6:           return KeyboardButton::F6;
		case XK_F7:           return KeyboardButton::F7;
		case XK_F8:           return KeyboardButton::F8;
		case XK_F9:           return KeyboardButton::F9;
		case XK_F10:          return KeyboardButton::F10;
		case XK_F11:          return KeyboardButton::F11;
		case XK_F12:          return KeyboardButton::F12;
		case XK_Home:         return KeyboardButton::HOME;
		case XK_Left:         return KeyboardButton::LEFT;
		case XK_Up:           return KeyboardButton::UP;
		case XK_Right:        return KeyboardButton::RIGHT;
		case XK_Down:         return KeyboardButton::DOWN;
		case XK_Page_Up:      return KeyboardButton::PAGE_UP;
		case XK_Page_Down:    return KeyboardButton::PAGE_DOWN;
		case XK_Insert:       return KeyboardButton::INSERT;
		case XK_Delete:       return KeyboardButton::DELETE;
		case XK_End:          return KeyboardButton::END;
		case XK_Shift_L:      return KeyboardButton::LEFT_SHIFT;
		case XK_Shift_R:      return KeyboardButton::RIGHT_SHIFT;
		case XK_Control_L:    return KeyboardButton::LEFT_CTRL;
		case XK_Control_R:    return KeyboardButton::RIGHT_CTRL;
		case XK_Caps_Lock:    return KeyboardButton::CAPS_LOCK;
		case XK_Alt_L:        return KeyboardButton::LEFT_ALT;
		case XK_Alt_R:        return KeyboardButton::RIGHT_ALT;
		case XK_Super_L:      return KeyboardButton::LEFT_SUPER;
		case XK_Super_R:      return KeyboardButton::RIGHT_SUPER;
		case XK_Num_Lock:     return KeyboardButton::NUM_LOCK;
		case XK_KP_Enter:     return KeyboardButton::NUMPAD_ENTER;
		case XK_KP_Delete:    return KeyboardButton::NUMPAD_DELETE;
		case XK_KP_Multiply:  return KeyboardButton::NUMPAD_MULTIPLY;
		case XK_KP_Add:       return KeyboardButton::NUMPAD_ADD;
		case XK_KP_Subtract:  return KeyboardButton::NUMPAD_SUBTRACT;
		case XK_KP_Divide:    return KeyboardButton::NUMPAD_DIVIDE;
		case XK_KP_Insert:
		case XK_KP_0:         return KeyboardButton::NUMPAD_0;
		case XK_KP_End:
		case XK_KP_1:         return KeyboardButton::NUMPAD_1;
		case XK_KP_Down:
		case XK_KP_2:         return KeyboardButton::NUMPAD_2;
		case XK_KP_Page_Down: // or XK_KP_Next
		case XK_KP_3:         return KeyboardButton::NUMPAD_3;
		case XK_KP_Left:
		case XK_KP_4:         return KeyboardButton::NUMPAD_4;
		case XK_KP_Begin:
		case XK_KP_5:         return KeyboardButton::NUMPAD_5;
		case XK_KP_Right:
		case XK_KP_6:         return KeyboardButton::NUMPAD_6;
		case XK_KP_Home:
		case XK_KP_7:         return KeyboardButton::NUMPAD_7;
		case XK_KP_Up:
		case XK_KP_8:         return KeyboardButton::NUMPAD_8;
		case XK_KP_Page_Up:   // or XK_KP_Prior
		case XK_KP_9:         return KeyboardButton::NUMPAD_9;
		case '0':             return KeyboardButton::NUMBER_0;
		case '1':             return KeyboardButton::NUMBER_1;
		case '2':             return KeyboardButton::NUMBER_2;
		case '3':             return KeyboardButton::NUMBER_3;
		case '4':             return KeyboardButton::NUMBER_4;
		case '5':             return KeyboardButton::NUMBER_5;
		case '6':             return KeyboardButton::NUMBER_6;
		case '7':             return KeyboardButton::NUMBER_7;
		case '8':             return KeyboardButton::NUMBER_8;
		case '9':             return KeyboardButton::NUMBER_9;
		case 'a':             return KeyboardButton::A;
		case 'b':             return KeyboardButton::B;
		case 'c':             return KeyboardButton::C;
		case 'd':             return KeyboardButton::D;
		case 'e':             return KeyboardButton::E;
		case 'f':             return KeyboardButton::F;
		case 'g':             return KeyboardButton::G;
		case 'h':             return KeyboardButton::H;
		case 'i':             return KeyboardButton::I;
		case 'j':             return KeyboardButton::J;
		case 'k':             return KeyboardButton::K;
		case 'l':             return KeyboardButton::L;
		case 'm':             return KeyboardButton::M;
		case 'n':             return KeyboardButton::N;
		case 'o':             return KeyboardButton::O;
		case 'p':             return KeyboardButton::P;
		case 'q':             return KeyboardButton::Q;
		case 'r':             return KeyboardButton::R;
		case 's':             return KeyboardButton::S;
		case 't':             return KeyboardButton::T;
		case 'u':             return KeyboardButton::U;
		case 'v':             return KeyboardButton::V;
		case 'w':             return KeyboardButton::W;
		case 'x':             return KeyboardButton::X;
		case 'y':             return KeyboardButton::Y;
		case 'z':             return KeyboardButton::Z;
		default:              return KeyboardButton::COUNT;
	}
}

#define JS_EVENT_BUTTON 0x01 // button pressed/released 
#define JS_EVENT_AXIS   0x02 // joystick moved 
#define JS_EVENT_INIT   0x80 // initial state of device 

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_THRESHOLD            30

static uint8_t joypadButtonList[] =
{
	JoypadButton::A,
	JoypadButton::B,
	JoypadButton::X,
	JoypadButton::Y,
	JoypadButton::LEFT_SHOULDER,
	JoypadButton::RIGHT_SHOULDER,
	JoypadButton::BACK,
	JoypadButton::START,
	JoypadButton::GUIDE,
	JoypadButton::LEFT_THUMB,
	JoypadButton::RIGHT_THUMB,
	JoypadButton::UP, // TODO (reported as axis)
	JoypadButton::DOWN,
	JoypadButton::LEFT,
	JoypadButton::RIGHT
};

static uint16_t deadZoneList[] =
{
	XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
	XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
	XINPUT_GAMEPAD_THRESHOLD,
	XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
	XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
	XINPUT_GAMEPAD_THRESHOLD
};

struct JoypadEvent
{
	uint32_t time;  // event timestamp in milliseconds
	int16_t value; // value 
	uint8_t type;   // event type 
	uint8_t number; // axis/button number
};

struct Joypad
{
	void init()
	{
		char jsPath[] = "/dev/input/jsX";
		char* indexStr = strchr(jspath, 'X');

		for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
		{
			*indexStr = '0' + i;
			fileDescriptorList[i] = open(jsPath, O_RDONLY | O_NONBLOCK);
		}

		memset(isConnectedList, 0, sizeof(isConnectedList));
		memset(axisDataList, 0, sizeof(axisDataList));
	}

	void shutdown()
	{
		for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
		{
			if (fileDescriptorList[i] != -1)
			{
				close(fileDescriptorList[i]);
			}
		}
	}

	void update(OsEventQueue& queue)
	{
		JoypadEvent ev;
		memset(&ev, 0, sizeof(ev));

		for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
		{
			const int fileDescriptor = fileDescriptorList[i];
			const bool connected = fileDescriptor != -1;

			if (connected != isConnectedList[i])
			{
				queue.pushJoypadEvent(i, connected);
			}

			isConnectedList[i] = connected;

			if (!connected)
			{
				continue;
			}

			while(read(fileDescriptor, &ev, sizeof(ev)) != -1)
			{
				const uint8_t number = ev.number;
				const int16_t val = ev.value;

				switch (ev.type &= ~JS_EVENT_INIT)
				{
					case JS_EVENT_AXIS:
					{
						AxisData& axis = axisDataList[i];
						// Indices into axis.left/right respectively
						const uint8_t axisIndexList[] = { 0, 1, 2, 0, 1, 2 };
						const int16_t deadZone = deadZoneList[number];

						int16_t value = val > deadZone || val < -deadZone ? val : 0;

						// Remap triggers to [0, INT16_MAX]
						if (number == 2 || number == 5)
						{
							value = (value + INT16_MAX) >> 1;
						}

						float* values = number > 2 ? axis.right : axis.left;

						values[axisIndexList[number]] = value != 0
							? float(value + (value < 0 ? deadZone : -deadZone)) / float(INT16_MAX - deadZone)
							: 0.0f
							;

						queue.pushJoypadEvent(i
							, number > 2 ? 1 : 0
							, values[0]
							, -values[1]
							, values[2]
							);
						break;
					}
					case JS_EVENT_BUTTON:
					{
						if (ev.number < RIO_COUNTOF(joypadButtonList))
						{
							queue.pushJoypadEvent(i
								, joypadButtonList[ev.number]
								, val == 1
								);
						}
						break;
					}
				}
			}
		}
	}

	int fileDescriptorList[RIO_MAX_JOYPADS];
	bool isConnectedList[RIO_MAX_JOYPADS];

	struct AxisData
	{
		float left[3];
		float right[3];
	};

	AxisData axisDataList[RIO_MAX_JOYPADS];
};

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

struct LinuxDevice
{
	LinuxDevice()
	{
	}

	int run(DeviceOptions* deviceOptions)
	{
		Status xStatus = XInitThreads();
		RIO_ASSERT(xStatus != 0, "XInitThreads: error");
		RIO_UNUSED(xStatus);

		x11Display = XOpenDisplay(NULL);
		RIO_ASSERT(x11Display != nullptr, "XOpenDisplay: error");

		::Window rootWindow = RootWindow(x11Display, DefaultScreen(x11Display));

		// Do we have detectable autorepeat?
		Bool detectable;
		x11DetectableAutorepeat = (bool)XkbSetDetectableAutoRepeat(x11Display, true, &detectable);

		wmDeleteMessage = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);

		// Save screen configuration
		screenConfig = XRRGetScreenInfo(x11Display, rootWindow);

		Rotation oldRotation;
		const SizeID rrOldSizeId = XRRConfigCurrentConfiguration(screenConfig, &oldRotation);

		// Start main thread
		MainThreadArgs mainThreadArgs;
		mainThreadArgs.deviceOptions = deviceOptions;

		Thread mainThread;
		mainThread.start(mainThreadFunction, &mainThreadArgs);

		joypad.init();

		while (exitIsRequested == false)
		{
			joypad.update(osEventQueue);

			while (XPending(x11Display))
			{
				XEvent event;
				XNextEvent(x11Display, &event);

				switch (event.type)
				{
					case EnterNotify:
					{
						osEventQueue.pushMouseEvent(event.xcrossing.x, event.xcrossing.y);
						break;
					}
					case ClientMessage:
					{
						if ((Atom)event.xclient.data.l[0] == wmDeleteMessage)
						{
							osEventQueue.pushExitEvent(0);
						}
						break;
					}
					case ConfigureNotify:
					{
						osEventQueue.pushMetricsEvent(event.xconfigure.x
							, event.xconfigure.y
							, event.xconfigure.width
							, event.xconfigure.height
							);
						break;
					}
					case ButtonPress:
					case ButtonRelease:
					{
						if (event.xbutton.button == Button4 || event.xbutton.button == Button5)
						{
							osEventQueue.pushMouseEvent(event.xbutton.x
								, event.xbutton.y
								, event.xbutton.button == Button4 ? 1.0f : -1.0f
								);
							break;
						}

						MouseButton::Enum mouseButton;
						switch (event.xbutton.button)
						{
							case Button1: mouseButton = MouseButton::LEFT; break;
							case Button2: mouseButton = MouseButton::MIDDLE; break;
							case Button3: mouseButton = MouseButton::RIGHT; break;
							default: mouseButton = MouseButton::COUNT; break;
						}

						if (mouseButton != MouseButton::COUNT)
						{
							osEventQueue.pushMouseEvent(event.xbutton.x
								, event.xbutton.y
								, mouseButton
								, event.type == ButtonPress
								);
						}
						break;
					}
					case MotionNotify:
					{
						osEventQueue.pushMouseEvent(event.xmotion.x, event.xmotion.y);
						break;
					}
					case KeyPress:
					case KeyRelease:
					{
						KeySym keySym = XLookupKeysym(&event.xkey, 0);
						KeyboardButton::Enum keyboardButton = getKeyboardButtonFromX11Key(keySym);

						if (keyboardButton != KeyboardButton::COUNT)
						{
							osEventQueue.pushKeyboardEvent(keyboardButton, event.type == KeyPress);
						}
						break;
					}
					case KeymapNotify:
					{
						XRefreshKeyboardMapping(&event.xmapping);
						break;
					}
					default:
					{
						break;
					}
				}
			}

			OsFn::sleep(16);
		}

		joypad.shutdown();

		mainThread.stop();

		// Restore previous screen configuration
		Rotation rotation;
		const SizeID rrSizeId = XRRConfigCurrentConfiguration(screenConfig, &rotation);

		if (rotation != oldRotation || rrSizeId != rrOldSizeId)
		{
			XRRSetScreenConfig(x11Display
				, screenConfig
				, rootWindow
				, rrOldSizeId
				, oldRotation
				, CurrentTime
				);
		}
		XRRFreeScreenConfigInfo(screenConfig);

		XCloseDisplay(x11Display);
		return EXIT_SUCCESS;
	}

public:
	::Display* x11Display = nullptr;
	Atom wmDeleteMessage;
	XRRScreenConfiguration* screenConfig;
	bool x11DetectableAutorepeat = false;
	OsEventQueue osEventQueue;
	Joypad joypad;
};

static LinuxDevice linuxDevice;

struct WindowX11 : public Window
{
public:
	WindowX11()
	{
		this->x11Display = linuxDevice.x11Display;
	}

	void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t parent)
	{
		int screen = DefaultScreen(x11Display);
		int depth = DefaultDepth(x11Display, screen);
		Visual* visual = DefaultVisual(x11Display, screen);

		::Window rootWindow = RootWindow(x11Display, screen);
		::Window parentWindow = (parent == 0) ? rootWindow : (::Window)parent;

		// Create main window
		XSetWindowAttributes windowAttributes;
		windowAttributes.background_pixmap = 0;
		windowAttributes.border_pixel = 0;
		windowAttributes.event_mask = FocusChangeMask
			| StructureNotifyMask
			;

		if (!parent)
		{
			windowAttributes.event_mask |= KeyPressMask
				| KeyReleaseMask
				| ButtonPressMask
				| ButtonReleaseMask
				| PointerMotionMask
				| EnterWindowMask
				;
		}
		else
		{
			XWindowAttributes parentAttributes;
			XGetWindowAttributes(x11Display, parentWindow, &parentAttributes);
			depth = parentAttributes.depth;
			visual = parentAttributes.visual;
		}

		x11Window = XCreateWindow(x11Display
			, parentWindow
			, x
			, y
			, width
			, height
			, 0
			, depth
			, InputOutput
			, visual
			, CWBorderPixel | CWEventMask
			, &windowAttributes
			);
		RIO_ASSERT(x11Window != None, "XCreateWindow: error");

		// Build hidden cursor
		Pixmap bitmapNoData;
		XColor black;
		XColor dummy;
		Colormap colormap;
		static char noData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		colormap = XDefaultColormap(x11Display, screen);
		XAllocNamedColor(x11Display, colormap, "black", &black, &dummy);
		bitmapNoData = XCreateBitmapFromData(x11Display, x11Window, noData, 8, 8);
		x11HiddenCursor = XCreatePixmapCursor(x11Display, bitmapNoData, bitmapNoData, &black, &black, 0, 0);

		wmDeleteMessage = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(x11Display, x11Window, &wmDeleteMessage, 1);

		XMapRaised(x11Display, x11Window);
	}

	void close()
	{
		XDestroyWindow(x11Display, x11Window);
	}

	void setupBgfx()
	{
		bgfx::x11SetDisplayWindow(x11Display, x11Window);
	}

	void show()
	{
		XMapRaised(x11Display, x11Window);
	}

	void hide()
	{
		XUnmapWindow(x11Display, x11Window);
	}

	void resize(uint16_t width, uint16_t height)
	{
		XResizeWindow(x11Display, x11Window, width, height);
	}

	void move(uint16_t x, uint16_t y)
	{
		XMoveWindow(x11Display, x11Window, x, y);
	}

	void minimize()
	{
	}

	void restore()
	{
	}

	const char* getTitle()
	{
		static char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		char* name;
		XFetchName(x11Display, x11Window, &name);
		strncpy(buffer, name, sizeof(buffer));
		XFree(name);
		return buffer;
	}

	void setTitle(const char* title)
	{
		XStoreName(x11Display, x11Window, title);
	}

	void* getHandle()
	{
		return (void*)(uintptr_t)x11Window;
	}

	void setShowCursor(bool show)
	{
		XDefineCursor(x11Display
			, x11Window
			, show ? None : x11HiddenCursor
			);
	}

	::Display* x11Display = nullptr;
	::Window x11Window = None;
	Cursor x11HiddenCursor = None;
	Atom wmDeleteMessage;
};

namespace WindowFn
{
	Window* create(Allocator& a)
	{
		return RIO_NEW(a, WindowX11)();
	}

	void destroy(Allocator& a, Window& window)
	{
		RIO_DELETE(a, &window);
	}
} // namespace WindowFn

struct DisplayXRandr : public Display
{
	DisplayXRandr()
	{
		this->x11Display = linuxDevice.x11Display;
		screenConfig = linuxDevice.screenConfig;
	}

	void getModes(Array<DisplayMode>& modes)
	{
		int modesCount = 0;
		XRRScreenSize* sizes = XRRConfigSizes(screenConfig, &modesCount);

		if (!sizes)
		{
			return;
		}

		for (int i = 0; i < modesCount; ++i)
		{
			DisplayMode displayMode;
			displayMode.id = (uint32_t)i;
			displayMode.width = sizes[i].width;
			displayMode.height = sizes[i].height;
			ArrayFn::pushBack(modes, displayMode);
		}
	}

	void setMode(uint32_t id)
	{
		int modesCount = 0;
		XRRScreenSize* sizes = XRRConfigSizes(screenConfig, &modesCount);

		if (!sizes || (int)id >= modesCount)
		{
			return;
		}

		XRRSetScreenConfig(x11Display
			, screenConfig
			, RootWindow(x11Display, DefaultScreen(x11Display))
			, (int)id
			, RR_Rotate_0
			, CurrentTime
			);
	}

	// void setFullscreen(bool isFullscreen)
	// {
	// 	XEvent e;
	// 	e.xclient.type = ClientMessage;
	// 	e.xclient.window = x11Window;
	// 	e.xclient.message_type = XInternAtom(x11Display, "_NET_WM_STATE", False );
	// 	e.xclient.format = 32;
	// 	e.xclient.data.l[0] = isFullscreen ? 1 : 0;
	// 	e.xclient.data.l[1] = XInternAtom(x11Display, "_NET_WM_STATE_FULLSCREEN", False);
	// 	XSendEvent(x11Display, DefaultRootWindow(x11Display), False, SubstructureNotifyMask, &e);
	// }

	::Display* x11Display = nullptr;
	XRRScreenConfiguration* screenConfig = nullptr;
};

namespace DisplayFn
{
	Display* create(Allocator& a)
	{
		return RIO_NEW(a, DisplayXRandr)();
	}

	void destroy(Allocator& a, Display& display)
	{
		RIO_DELETE(a, &display);
	}
} // namespace DisplayFn

bool getNextEvent(OsEvent& ev)
{
	return linuxDevice.osEventQueue.popEvent(ev);
}

} // namespace Rio

struct InitMemoryGlobals
{
	InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::init();
	}

	~InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::shutdown();
	}
};

int main(int argumentListCount, char** argumentList)
{
	using namespace Rio;
#if RIO_BUILD_UNIT_TESTS
	CommandLine commandLine(argumentListCount, (const char**)argumentList);
	if (commandLine.hasArgument("runUnitTests"))
	{
		runUnitTests();
		return EXIT_SUCCESS;
	}
#endif // RIO_BUILD_UNIT_TESTS
	InitMemoryGlobals initMemoryGlobals;
	RIO_UNUSED(initMemoryGlobals);

	DeviceOptions deviceOptions(argumentListCount, (const char**)argumentList);
	if (deviceOptions.parse() == EXIT_SUCCESS)
	{
		return linuxDevice.run(&deviceOptions);
	}

	return EXIT_FAILURE;
}

#endif // RIO_PLATFORM_LINUX
// Copyright (c) 2016 Volodymyr Syvochka