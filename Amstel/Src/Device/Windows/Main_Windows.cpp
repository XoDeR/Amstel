// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_WINDOWS

#include "Core/Base/CommandLine.h"
#include "Core/Thread/Thread.h"
#include "Device/Device.h"
#include "Device/Display.h"
#include "Device/DeviceEventQueue.h"
#include "Device/Windows/Headers_Windows.h"

#include "Core/UnitTests.cpp"

#include <bgfx/bgfxplatform.h>

#include <WindowsX.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

namespace Rio
{

static KeyboardButton::Enum getKeyFromWindowsKeyId(int32_t windowsKeyId)
{
	switch (windowsKeyId)
	{
		case VK_BACK:     return KeyboardButton::BACKSPACE;
		case VK_TAB:      return KeyboardButton::TAB;
		case VK_SPACE:    return KeyboardButton::SPACE;
		case VK_ESCAPE:   return KeyboardButton::ESCAPE;
		case VK_RETURN:   return KeyboardButton::ENTER;
		case VK_F1:       return KeyboardButton::F1;
		case VK_F2:       return KeyboardButton::F2;
		case VK_F3:       return KeyboardButton::F3;
		case VK_F4:       return KeyboardButton::F4;
		case VK_F5:       return KeyboardButton::F5;
		case VK_F6:       return KeyboardButton::F6;
		case VK_F7:       return KeyboardButton::F7;
		case VK_F8:       return KeyboardButton::F8;
		case VK_F9:       return KeyboardButton::F9;
		case VK_F10:      return KeyboardButton::F10;
		case VK_F11:      return KeyboardButton::F11;
		case VK_F12:      return KeyboardButton::F12;
		case VK_HOME:     return KeyboardButton::HOME;
		case VK_LEFT:     return KeyboardButton::LEFT;
		case VK_UP:       return KeyboardButton::UP;
		case VK_RIGHT:    return KeyboardButton::RIGHT;
		case VK_DOWN:     return KeyboardButton::DOWN;
		case VK_PRIOR:    return KeyboardButton::PAGE_UP;
		case VK_NEXT:     return KeyboardButton::PAGE_DOWN;
		case VK_INSERT:   return KeyboardButton::INSERT;
		case VK_DELETE:   return KeyboardButton::DELETE;
		case VK_END:      return KeyboardButton::END;
		case VK_LSHIFT:   return KeyboardButton::LEFT_SHIFT;
		case VK_RSHIFT:   return KeyboardButton::RIGHT_SHIFT;
		case VK_LCONTROL: return KeyboardButton::LEFT_CTRL;
		case VK_RCONTROL: return KeyboardButton::RIGHT_CTRL;
		case VK_CAPITAL:  return KeyboardButton::CAPS_LOCK;
		case VK_LMENU:    return KeyboardButton::LEFT_ALT;
		case VK_RMENU:    return KeyboardButton::RIGHT_ALT;
		case VK_LWIN:     return KeyboardButton::LEFT_SUPER;
		case VK_RWIN:     return KeyboardButton::RIGHT_SUPER;
		case VK_NUMLOCK:  return KeyboardButton::NUM_LOCK;
		// case VK_RETURN:   return KeyboardButton::NUMPAD_ENTER;
		case VK_DECIMAL:  return KeyboardButton::NUMPAD_DELETE;
		case VK_MULTIPLY: return KeyboardButton::NUMPAD_MULTIPLY;
		case VK_ADD:      return KeyboardButton::NUMPAD_ADD;
		case VK_SUBTRACT: return KeyboardButton::NUMPAD_SUBTRACT;
		case VK_DIVIDE:   return KeyboardButton::NUMPAD_DIVIDE;
		case VK_NUMPAD0:  return KeyboardButton::NUMPAD_0;
		case VK_NUMPAD1:  return KeyboardButton::NUMPAD_1;
		case VK_NUMPAD2:  return KeyboardButton::NUMPAD_2;
		case VK_NUMPAD3:  return KeyboardButton::NUMPAD_3;
		case VK_NUMPAD4:  return KeyboardButton::NUMPAD_4;
		case VK_NUMPAD5:  return KeyboardButton::NUMPAD_5;
		case VK_NUMPAD6:  return KeyboardButton::NUMPAD_6;
		case VK_NUMPAD7:  return KeyboardButton::NUMPAD_7;
		case VK_NUMPAD8:  return KeyboardButton::NUMPAD_8;
		case VK_NUMPAD9:  return KeyboardButton::NUMPAD_9;
		case '0':         return KeyboardButton::NUMBER_0;
		case '1':         return KeyboardButton::NUMBER_1;
		case '2':         return KeyboardButton::NUMBER_2;
		case '3':         return KeyboardButton::NUMBER_3;
		case '4':         return KeyboardButton::NUMBER_4;
		case '5':         return KeyboardButton::NUMBER_5;
		case '6':         return KeyboardButton::NUMBER_6;
		case '7':         return KeyboardButton::NUMBER_7;
		case '8':         return KeyboardButton::NUMBER_8;
		case '9':         return KeyboardButton::NUMBER_9;
		case 'A':         return KeyboardButton::A;
		case 'B':         return KeyboardButton::B;
		case 'C':         return KeyboardButton::C;
		case 'D':         return KeyboardButton::D;
		case 'E':         return KeyboardButton::E;
		case 'F':         return KeyboardButton::F;
		case 'G':         return KeyboardButton::G;
		case 'H':         return KeyboardButton::H;
		case 'I':         return KeyboardButton::I;
		case 'J':         return KeyboardButton::J;
		case 'K':         return KeyboardButton::K;
		case 'L':         return KeyboardButton::L;
		case 'M':         return KeyboardButton::M;
		case 'N':         return KeyboardButton::N;
		case 'O':         return KeyboardButton::O;
		case 'P':         return KeyboardButton::P;
		case 'Q':         return KeyboardButton::Q;
		case 'R':         return KeyboardButton::R;
		case 'S':         return KeyboardButton::S;
		case 'T':         return KeyboardButton::T;
		case 'U':         return KeyboardButton::U;
		case 'V':         return KeyboardButton::V;
		case 'W':         return KeyboardButton::W;
		case 'X':         return KeyboardButton::X;
		case 'Y':         return KeyboardButton::Y;
		case 'Z':         return KeyboardButton::Z;
		default:          return KeyboardButton::COUNT;
	}
}

struct XinputToJoypad
{
	WORD bit;
	JoypadButton::Enum button;
};

static XinputToJoypad xInputToJoypadTable[] =
{
	{ XINPUT_GAMEPAD_DPAD_UP,        JoypadButton::UP             },
	{ XINPUT_GAMEPAD_DPAD_DOWN,      JoypadButton::DOWN           },
	{ XINPUT_GAMEPAD_DPAD_LEFT,      JoypadButton::LEFT           },
	{ XINPUT_GAMEPAD_DPAD_RIGHT,     JoypadButton::RIGHT          },
	{ XINPUT_GAMEPAD_START,          JoypadButton::START          },
	{ XINPUT_GAMEPAD_BACK,           JoypadButton::BACK           },
	{ XINPUT_GAMEPAD_LEFT_THUMB,     JoypadButton::LEFT_THUMB     },
	{ XINPUT_GAMEPAD_RIGHT_THUMB,    JoypadButton::RIGHT_THUMB    },
	{ XINPUT_GAMEPAD_LEFT_SHOULDER,  JoypadButton::LEFT_SHOULDER  },
	{ XINPUT_GAMEPAD_RIGHT_SHOULDER, JoypadButton::RIGHT_SHOULDER },
	{ XINPUT_GAMEPAD_A,              JoypadButton::A              },
	{ XINPUT_GAMEPAD_B,              JoypadButton::B              },
	{ XINPUT_GAMEPAD_X,              JoypadButton::X              },
	{ XINPUT_GAMEPAD_Y,              JoypadButton::Y              }
};

struct Joypad
{
	void init()
	{
		memset(&stateList, 0, sizeof(stateList));
		memset(&axisDataList, 0, sizeof(axisDataList));
		memset(&isConnectedList, 0, sizeof(isConnectedList));
	}

	void update(DeviceEventQueue& queue)
	{
		for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
		{
			XINPUT_STATE state;
			memset(&state, 0, sizeof(state));

			const DWORD result = XInputGetState(i, &state);
			const bool connected = result == ERROR_SUCCESS;

			if (connected != isConnectedList[i])
			{
				queue.pushStatusEvent(InputDeviceType::JOYPAD, i, connected);
			}

			isConnectedList[i] = connected;

			if (!connected || state.dwPacketNumber == stateList[i].dwPacketNumber)
			{
				continue;
			}

			XINPUT_GAMEPAD& gamepad = stateList[i].Gamepad;

			const WORD diff = state.Gamepad.wButtons ^ gamepad.wButtons;
			const WORD current = state.Gamepad.wButtons;
			if (diff != 0)
			{
				for (uint8_t bb = 0; bb < RIO_COUNTOF(xInputToJoypadTable); ++bb)
				{
					WORD bit = xInputToJoypadTable[bb].bit;
					if (bit & diff)
					{
						queue.pushButtonEvent(InputDeviceType::JOYPAD
							, i
							, xInputToJoypadTable[bb].button
							, (current & bit) != 0);
						gamepad.wButtons = current;
					}
				}
			}

			if (state.Gamepad.sThumbLX != gamepad.sThumbLX)
			{
				SHORT value = state.Gamepad.sThumbLX;
				value = value > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
					? value : 0;

				axisDataList[0].lx = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, axisDataList[0].lx
					, axisDataList[0].ly
					, axisDataList[0].lz
					);

				gamepad.sThumbLX = state.Gamepad.sThumbLX;
			}
			if (state.Gamepad.sThumbLY != gamepad.sThumbLY)
			{
				SHORT value = state.Gamepad.sThumbLY;
				value = value > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
					? value : 0;

				axisDataList[0].ly = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i 
					, JoypadAxis::LEFT
					, axisDataList[0].lx
					, axisDataList[0].ly
					, axisDataList[0].lz
					);

				gamepad.sThumbLY = state.Gamepad.sThumbLY;
			}
			if (state.Gamepad.bLeftTrigger != gamepad.bLeftTrigger)
			{
				BYTE value = state.Gamepad.bLeftTrigger;
				value = value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? value : 0;

				axisDataList[0].lz = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_TRIGGER_THRESHOLD : -XINPUT_GAMEPAD_TRIGGER_THRESHOLD)) / float(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, axisDataList[0].lx
					, axisDataList[0].ly
					, axisDataList[0].lz
					);

				gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
			}
			if (state.Gamepad.sThumbRX != gamepad.sThumbRX)
			{
				SHORT value = state.Gamepad.sThumbRX;
				value = value > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
					? value : 0;

				axisDataList[0].rx = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, axisDataList[0].rx
					, axisDataList[0].ry
					, axisDataList[0].rz
					);

				gamepad.sThumbRX = state.Gamepad.sThumbRX;
			}
			if (state.Gamepad.sThumbRY != gamepad.sThumbRY)
			{
				SHORT value = state.Gamepad.sThumbRY;
				value = value > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
					? value : 0;

				axisDataList[0].ry = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, axisDataList[0].rx
					, axisDataList[0].ry
					, axisDataList[0].rz
					);

				gamepad.sThumbRY = state.Gamepad.sThumbRY;
			}
			if (state.Gamepad.bRightTrigger != gamepad.bRightTrigger)
			{
				BYTE value = state.Gamepad.bRightTrigger;
				value = value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? value : 0;

				axisDataList[0].rz = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_TRIGGER_THRESHOLD : -XINPUT_GAMEPAD_TRIGGER_THRESHOLD)) / float(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					: 0.0f
					;
				queue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, axisDataList[0].rx
					, axisDataList[0].ry
					, axisDataList[0].rz
					);

				gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
			}
		}
	}

	struct Axis
	{
		float lx;
		float ly;
		float lz;
		float rx;
		float ry;
		float rz;
	};

	XINPUT_STATE stateList[RIO_MAX_JOYPADS];
	Axis axisDataList[RIO_MAX_JOYPADS];
	bool isConnectedList[RIO_MAX_JOYPADS];
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

struct WindowsDevice
{
	int	run(DeviceOptions* deviceOptions)
	{
		MainThreadArgs mainThreadArgs;
		mainThreadArgs.deviceOptions = deviceOptions;

		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		WNDCLASSEX wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc = windowProcedure;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(instance, IDC_ARROW);
		wnd.lpszClassName = "Rio";
		wnd.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		windowHandle = CreateWindowA("Rio"
			, "Rio"
			, WS_OVERLAPPEDWINDOW | WS_VISIBLE
			, deviceOptions->windowX
			, deviceOptions->windowY
			, deviceOptions->windowWidth
			, deviceOptions->windowHeight
			, 0
			, NULL
			, instance
			, 0
			);
		RIO_ASSERT(windowHandle != NULL, "CreateWindowA: GetLastError = %d", GetLastError());

		Thread mainThread;
		mainThread.start(mainThreadFunction, &mainThreadArgs);

		MSG msg;
		msg.message = WM_NULL;

		while (exitIsRequested == false)
		{
			joypad.update(deviceEventQueue);

			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		mainThread.stop();
		DestroyWindow(windowHandle);
		return EXIT_SUCCESS;
	}

	LRESULT pumpEvents(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		switch (id)
		{
			case WM_DESTROY:
			{
				
			}
			break;
			case WM_QUIT:
			case WM_CLOSE:
			{
				exitIsRequested = true;
				deviceEventQueue.pushExitEvent();
				return 0;
			}
			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushResolutionEvent(width, height);
			}
			break;
			case WM_SYSCOMMAND:
			{
				switch (wparam)
				{
					case SC_MINIMIZE:
					case SC_RESTORE:
					{
						HWND parent = GetWindow(hwnd, GW_OWNER);
						if (NULL != parent)
						{
							PostMessage(parent, id, wparam, lparam);
						}
					}
					break;
				}
			}
			break;
			case WM_MOUSEWHEEL:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				short delta = GET_WHEEL_DELTA_WPARAM(wparam);
				deviceEventQueue.pushAxisEvent(InputDeviceType::MOUSE
					, 0
					, MouseAxis::WHEEL
					, 0.0f
					, (float)(delta/WHEEL_DELTA)
					, 0.0f
					);
			}
			break;
			case WM_MOUSEMOVE:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushAxisEvent(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR
					, mx
					, my
					, 0.0f
					);
			}
			break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::LEFT
					, id == WM_LBUTTONDOWN
					);
				break;
			}
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::RIGHT
					, id == WM_RBUTTONDOWN
					);
				break;
			}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::MIDDLE
					, id == WM_MBUTTONDOWN
					);
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				KeyboardButton::Enum key = getKeyFromWindowsKeyId(wparam & 0xff);
				if (key != KeyboardButton::COUNT)
				{
					deviceEventQueue.pushButtonEvent(InputDeviceType::KEYBOARD
						, 0
						, key
						, (id == WM_KEYDOWN || id == WM_SYSKEYDOWN)
						);
				}
			}
			break;
			default:
				break;
		}

		return DefWindowProc(hwnd, id, wparam, lparam);
	}

	static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

public:
	HWND windowHandle = nullptr;
	DeviceEventQueue deviceEventQueue;
	Joypad joypad;
};

static WindowsDevice windowsDevice;

LRESULT CALLBACK WindowsDevice::windowProcedure(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return windowsDevice.pumpEvents(hwnd, id, wparam, lparam);
}

struct Window_Windows : public Window
{
	Window_Windows()
	{
		windowHandle = windowsDevice.windowHandle;
	}

	void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t /*parent*/)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	void close()
	{
	}

	void setupBgfx()
	{
		bgfx::winSetHwnd(windowHandle);
	}

	void show()
	{
		ShowWindow(windowHandle, SW_SHOW);
	}

	void hide()
	{
		ShowWindow(windowHandle, SW_HIDE);
	}

	void resize(uint16_t width, uint16_t height)
	{
		this->width = width;
		this->height = height;
		MoveWindow(windowHandle, this->x, this->y, width, height, FALSE);
	}

	void move(uint16_t x, uint16_t y)
	{
		this->x = x;
		this->y = y;
		MoveWindow(windowHandle, x, y, width, height, FALSE);
	}

	void minimize()
	{
		ShowWindow(windowHandle, SW_MINIMIZE);
	}

	void restore()
	{
		ShowWindow(windowHandle, SW_RESTORE);
	}

	const char* getTitle()
	{
		static char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		GetWindowText(windowHandle, buffer, sizeof(buffer));
		return buffer;
	}

	void setTitle(const char* title)
	{
		SetWindowText(windowHandle, title);
	}

	void setShowCursor(bool show)
	{
		ShowCursor(show);
	}

	void setFullscreen(bool /*isFullscreen*/)
	{
	}

	void* getHandle()
	{
		return (void*)(uintptr_t)windowHandle;
	}

	HWND windowHandle = nullptr;
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t width = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t height = RIO_DEFAULT_WINDOW_HEIGHT;
};

namespace WindowFn
{
	Window* create(Allocator& a)
	{
		return RIO_NEW(a, Window_Windows)();
	}

	void destroy(Allocator& a, Window& window)
	{
		RIO_DELETE(a, &window);
	}
} // namespace WindowFn

struct Display_Windows : public Display
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
		return RIO_NEW(a, Display_Windows)();
	}

	void destroy(Allocator& a, Display& display)
	{
		RIO_DELETE(a, &display);
	}
} // namespace DisplayFn

bool getNextEvent(OsEvent& ev)
{
	return windowsDevice.deviceEventQueue.popEvent(ev);
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

	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	RIO_ASSERT(err == 0, "WSAStartup: error = %d", err);
	RIO_UNUSED(wsaData);
	RIO_UNUSED(err);

	DeviceOptions deviceOptions(argumentListCount, (const char**)argumentList);
	if (deviceOptions.parse() != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	return windowsDevice.run(&deviceOptions);
}

#endif // RIO_PLATFORM_WINDOWS
// Copyright (c) 2016 Volodymyr Syvochka