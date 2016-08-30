// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

namespace Rio
{

class InputManager;
struct InputDevice;

struct InputDeviceType
{
	enum Enum
	{
		KEYBOARD,
		MOUSE,
		TOUCHSCREEN,
		JOYPAD
	};
};

struct KeyboardButton
{
	enum Enum
	{
		TAB,
		ENTER,
		ESCAPE,
		SPACE,
		BACKSPACE,

		// Numpad
		NUM_LOCK,
		NUMPAD_ENTER,
		NUMPAD_DELETE,
		NUMPAD_MULTIPLY,
		NUMPAD_ADD,
		NUMPAD_SUBTRACT,
		NUMPAD_DIVIDE,
		NUMPAD_0,
		NUMPAD_1,
		NUMPAD_2,
		NUMPAD_3,
		NUMPAD_4,
		NUMPAD_5,
		NUMPAD_6,
		NUMPAD_7,
		NUMPAD_8,
		NUMPAD_9,

		// Function keys
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		// Other keys
		HOME,
		LEFT,
		UP,
		RIGHT,
		DOWN,
		PAGE_UP,
		PAGE_DOWN,
		INSERT,
		DELETE,
		END,

		// Modifier keys
		LEFT_CTRL,
		RIGHT_CTRL,
		LEFT_SHIFT,
		RIGHT_SHIFT,
		CAPS_LOCK,
		LEFT_ALT,
		RIGHT_ALT,
		LEFT_SUPER,
		RIGHT_SUPER,

		NUMBER_0,
		NUMBER_1,
		NUMBER_2,
		NUMBER_3,
		NUMBER_4,
		NUMBER_5,
		NUMBER_6,
		NUMBER_7,
		NUMBER_8,
		NUMBER_9,

		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		COUNT
	};
};

struct MouseButton
{
	enum Enum
	{
		LEFT,
		MIDDLE,
		RIGHT,
		EXTRA_1,
		EXTRA_2,
		COUNT
	};
};

struct MouseAxis
{
	enum Enum
	{
		CURSOR,
		CURSOR_DELTA,
		WHEEL,
		COUNT
	};
};

struct TouchButton
{
	enum Enum
	{
		POINTER_0,
		POINTER_1,
		POINTER_2,
		POINTER_3,
		COUNT
	};
};

struct TouchAxis
{
	enum Enum
	{
		POINTER_0,
		POINTER_1,
		POINTER_2,
		POINTER_3,
		COUNT
	};
};

struct JoypadButton
{
	enum Enum
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		START,
		BACK,
		GUIDE,
		LEFT_THUMB,
		RIGHT_THUMB,
		LEFT_SHOULDER,
		RIGHT_SHOULDER,
		A,
		B,
		X,
		Y,
		COUNT
	};
};

struct JoypadAxis
{
	enum Enum
	{
		LEFT,
		RIGHT,
		COUNT
	};
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka