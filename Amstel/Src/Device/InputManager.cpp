// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/InputManager.h"
#include "Device/InputDevice.h"
#include "Core/Base/Macros.h"
#include "Core/Memory/Memory.h"

namespace Rio
{

static const char* keyboardButtonNameList[] =
{
	"tab",          // KeyboardButton::TAB
	"enter",        // KeyboardButton::ENTER
	"escape",       // KeyboardButton::ESCAPE
	"space",        // KeyboardButton::SPACE
	"backspace",    // KeyboardButton::BACKSPACE
	"num_lock",     // KeyboardButton::NUM_LOCK
	"numpad_enter", // KeyboardButton::NUMPAD_ENTER
	"numpad_.",     // KeyboardButton::NUMPAD_DELETE
	"numpad_*",     // KeyboardButton::NUMPAD_MULTIPLY
	"numpad_+",     // KeyboardButton::NUMPAD_ADD
	"numpad_-",     // KeyboardButton::NUMPAD_SUBTRACT
	"numpad_/",     // KeyboardButton::NUMPAD_DIVIDE
	"numpad_0",     // KeyboardButton::NUMPAD_0
	"numpad_1",     // KeyboardButton::NUMPAD_1
	"numpad_2",     // KeyboardButton::NUMPAD_2
	"numpad_3",     // KeyboardButton::NUMPAD_3
	"numpad_4",     // KeyboardButton::NUMPAD_4
	"numpad_5",     // KeyboardButton::NUMPAD_5
	"numpad_6",     // KeyboardButton::NUMPAD_6
	"numpad_7",     // KeyboardButton::NUMPAD_7
	"numpad_8",     // KeyboardButton::NUMPAD_8
	"numpad_9",     // KeyboardButton::NUMPAD_9
	"f1",           // KeyboardButton::F1
	"f2",           // KeyboardButton::F2
	"f3",           // KeyboardButton::F3
	"f4",           // KeyboardButton::F4
	"f5",           // KeyboardButton::F5
	"f6",           // KeyboardButton::F6
	"f7",           // KeyboardButton::F7
	"f8",           // KeyboardButton::F8
	"f9",           // KeyboardButton::F9
	"f10",          // KeyboardButton::F10
	"f11",          // KeyboardButton::F11
	"f12",          // KeyboardButton::F12
	"home",         // KeyboardButton::HOME
	"left",         // KeyboardButton::LEFT
	"up",           // KeyboardButton::UP
	"right",        // KeyboardButton::RIGHT
	"down",         // KeyboardButton::DOWN
	"page_up",      // KeyboardButton::PAGE_UP
	"page_down",    // KeyboardButton::PAGE_DOWN
	"insert",       // KeyboardButton::INSERT
	"delete",       // KeyboardButton::DELETE
	"end",          // KeyboardButton::END
	"left_ctrl",    // KeyboardButton::LEFT_CTRL
	"right_ctrl",   // KeyboardButton::RIGHT_CTRL
	"left_shift",   // KeyboardButton::LEFT_SHIFT
	"right_shift",  // KeyboardButton::RIGHT_SHIFT
	"caps_lock",    // KeyboardButton::CAPS_LOCK
	"left_alt",     // KeyboardButton::LEFT_ALT
	"right_alt",    // KeyboardButton::RIGHT_ALT
	"left_super",   // KeyboardButton::LEFT_SUPER
	"right_super",  // KeyboardButton::RIGHT_SUPER
	"0",            // KeyboardButton::NUMBER_0
	"1",            // KeyboardButton::NUMBER_1
	"2",            // KeyboardButton::NUMBER_2
	"3",            // KeyboardButton::NUMBER_3
	"4",            // KeyboardButton::NUMBER_4
	"5",            // KeyboardButton::NUMBER_5
	"6",            // KeyboardButton::NUMBER_6
	"7",            // KeyboardButton::NUMBER_7
	"8",            // KeyboardButton::NUMBER_8
	"9",            // KeyboardButton::NUMBER_9
	"a",            // KeyboardButton::A
	"b",            // KeyboardButton::B
	"c",            // KeyboardButton::C
	"d",            // KeyboardButton::D
	"e",            // KeyboardButton::E
	"f",            // KeyboardButton::F
	"g",            // KeyboardButton::G
	"h",            // KeyboardButton::H
	"i",            // KeyboardButton::I
	"j",            // KeyboardButton::J
	"k",            // KeyboardButton::K
	"l",            // KeyboardButton::L
	"m",            // KeyboardButton::M
	"n",            // KeyboardButton::N
	"o",            // KeyboardButton::O
	"p",            // KeyboardButton::P
	"q",            // KeyboardButton::Q
	"r",            // KeyboardButton::R
	"s",            // KeyboardButton::S
	"t",            // KeyboardButton::T
	"u",            // KeyboardButton::U
	"v",            // KeyboardButton::V
	"w",            // KeyboardButton::W
	"x",            // KeyboardButton::X
	"y",            // KeyboardButton::Y
	"z"             // KeyboardButton::Z
};
RIO_STATIC_ASSERT(RIO_COUNTOF(keyboardButtonNameList) == KeyboardButton::COUNT);

static const char* mouseButtonNameList[] =
{
	"left",    // MouseButton::LEFT
	"middle",  // MouseButton::MIDDLE
	"right",   // MouseButton::RIGHT
	"extra_1", // MouseButton::EXTRA_1
	"extra_2"  // MouseButton::EXTRA_2
};
RIO_STATIC_ASSERT(RIO_COUNTOF(mouseButtonNameList) == MouseButton::COUNT);

static const char* mouseAxisNameList[] =
{
	"cursor",       // MouseAxis::CURSOR
	"cursor_delta", // MouseAxis::CURSOR_DELTA
	"wheel"         // MouseAxis::WHEEL
};
RIO_STATIC_ASSERT(RIO_COUNTOF(mouseAxisNameList) == MouseAxis::COUNT);

static const char* touchButtonNameList[] =
{
	"pointer_0", // TouchButton::POINTER_0
	"pointer_1", // TouchButton::POINTER_1
	"pointer_2", // TouchButton::POINTER_2
	"pointer_3"  // TouchButton::POINTER_3
};
RIO_STATIC_ASSERT(RIO_COUNTOF(touchButtonNameList) == TouchButton::COUNT);

static const char* touchAxisNameList[] =
{
	"pointer_0", // TouchAxis::POINTER_0
	"pointer_1", // TouchAxis::POINTER_1
	"pointer_2", // TouchAxis::POINTER_2
	"pointer_3"  // TouchAxis::POINTER_3
};
RIO_STATIC_ASSERT(RIO_COUNTOF(touchAxisNameList) == TouchAxis::COUNT);


static const char* joypadButtonNameList[] =
{
	"up",             // JoypadButton::UP
	"down",           // JoypadButton::DOWN
	"left",           // JoypadButton::LEFT
	"right",          // JoypadButton::RIGHT
	"start",          // JoypadButton::START
	"back",           // JoypadButton::BACK
	"guide",          // JoypadButton::GUIDE
	"left_thumb",     // JoypadButton::LEFT_THUMB
	"right_thumb",    // JoypadButton::RIGHT_THUMB
	"left_shoulder",  // JoypadButton::LEFT_SHOULDER
	"right_shoulder", // JoypadButton::RIGHT_SHOULDER
	"a",              // JoypadButton::A
	"b",              // JoypadButton::B
	"x",              // JoypadButton::X
	"y"               // JoypadButton::Y
};
RIO_STATIC_ASSERT(RIO_COUNTOF(joypadButtonNameList) == JoypadButton::COUNT);

static const char* joypadAxisNameList[] =
{
	"left",  // JoypadAxis::LEFT
	"right"  // JoypadAxis::RIGHT
};
RIO_STATIC_ASSERT(RIO_COUNTOF(joypadAxisNameList) == JoypadAxis::COUNT);

InputManager::InputManager(Allocator& a)
	: allocator(&a)
{
	keyboard = InputDeviceFn::create(*allocator
		, "Keyboard"
		, KeyboardButton::COUNT
		, 0
		, keyboardButtonNameList
		, nullptr
		);
	mouse = InputDeviceFn::create(*allocator
		, "Mouse"
		, MouseButton::COUNT
		, MouseAxis::COUNT
		, mouseButtonNameList
		, mouseAxisNameList
		);
	touch = InputDeviceFn::create(*allocator
		, "Touch"
		, TouchButton::COUNT
		, TouchAxis::COUNT
		, touchButtonNameList
		, touchAxisNameList
		);

	for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
	{
		joypadList[i] = InputDeviceFn::create(*allocator
			, "Joypad"
			, JoypadButton::COUNT
			, JoypadAxis::COUNT
			, joypadButtonNameList
			, joypadAxisNameList
			);
	}

	keyboard->setIsConnected(true);
	mouse->setIsConnected(true);
	touch->setIsConnected(true);
}

InputManager::~InputManager()
{
	for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
	{
		InputDeviceFn::destroy(*allocator, *joypadList[i]);
	}

	InputDeviceFn::destroy(*allocator, *touch);
	InputDeviceFn::destroy(*allocator, *mouse);
	InputDeviceFn::destroy(*allocator, *keyboard);
}

InputDevice* InputManager::getKeyboard()
{
	return keyboard;
}

InputDevice* InputManager::getMouse()
{
	return mouse;
}

InputDevice* InputManager::getTouch()
{
	return touch;
}

uint8_t InputManager::getJoypadListCount()
{
	return RIO_COUNTOF(joypadList);
}

InputDevice* InputManager::getJoypad(uint8_t i)
{
	RIO_ASSERT(i < RIO_MAX_JOYPADS, "Index out of bounds");
	return joypadList[i];
}

void InputManager::update()
{
	keyboard->update();
	mouse->update();
	touch->update();

	for (uint8_t i = 0; i < RIO_MAX_JOYPADS; ++i)
	{
		joypadList[i]->update();
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka