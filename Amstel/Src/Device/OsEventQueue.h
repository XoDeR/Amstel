// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Device/InputTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Thread/AtomicInt.h"

namespace Rio
{

struct OsMetricsEvent
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

struct OsExitEvent
{
	int32_t code;
};

struct OsMouseEvent
{
	enum Enum
	{
		BUTTON,
		WHEEL,
		MOVE
	};

	OsMouseEvent::Enum type;
	MouseButton::Enum button;
	int16_t x;
	int16_t y;
	bool pressed;
	float wheel;
};

struct OsKeyboardEvent
{
	KeyboardButton::Enum button;
	bool pressed;
};

struct OsTouchEvent
{
	enum Enum
	{
		POINTER,
		MOVE
	};

	OsTouchEvent::Enum type;
	uint8_t pointerId;
	int16_t x;
	int16_t y;
	bool pressed;
};

struct OsJoypadEvent
{
	enum Enum
	{
		BUTTON,
		AXIS,
		CONNECTED
	};

	OsJoypadEvent::Enum type;
	uint8_t index;
	bool connected;
	uint8_t button;
	bool pressed;
	float x;
	float y;
	float z;
};

struct OsAccelerometerEvent
{
	float x;
	float y;
	float z;
};

struct OsEvent
{
	enum Enum
	{
		NONE,

		KEYBOARD,
		MOUSE,
		TOUCH,
		JOYPAD,
		ACCELEROMETER,

		METRICS,
		PAUSE,
		RESUME,
		// Exit from program
		EXIT
	};

	OsEvent::Enum type;
	union
	{
		OsMetricsEvent metrics;
		OsExitEvent exit;
		OsMouseEvent mouse;
		OsKeyboardEvent keyboard;
		OsTouchEvent touch;
		OsJoypadEvent joypad;
		OsAccelerometerEvent accelerometer;
	};
};

#define MAX_OS_EVENTS 4096

// Single Producer Single Consumer event queue
// Used only to pass events from os thread to main thread
struct OsEventQueue
{
	OsEventQueue()
	{
	}

	void pushMouseEvent(uint16_t x, uint16_t y)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::MOVE;
		ev.mouse.x = x;
		ev.mouse.y = y;

		pushEvent(ev);
	}

	void pushMouseEvent(int16_t x, int16_t y, MouseButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::BUTTON;
		ev.mouse.x = x;
		ev.mouse.y = y;
		ev.mouse.button = b;
		ev.mouse.pressed = pressed;

		pushEvent(ev);
	}

	void pushMouseEvent(int16_t x, int16_t y, float wheel)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::WHEEL;
		ev.mouse.x = x;
		ev.mouse.y = y;
		ev.mouse.wheel = wheel;

		pushEvent(ev);
	}

	void pushKeyboardEvent(KeyboardButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::KEYBOARD;
		ev.keyboard.button = b;
		ev.keyboard.pressed = pressed;

		pushEvent(ev);
	}

	void pushTouchEvent(int16_t x, int16_t y, uint8_t pointerId)
	{
		OsEvent ev;
		ev.type = OsEvent::TOUCH;
		ev.touch.type = OsTouchEvent::MOVE;
		ev.touch.x = x;
		ev.touch.y = y;
		ev.touch.pointerId = pointerId;

		pushEvent(ev);
	}

	void pushTouchEvent(int16_t x, int16_t y, uint8_t pointerId, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::TOUCH;
		ev.touch.type = OsTouchEvent::POINTER;
		ev.touch.x = x;
		ev.touch.y = y;
		ev.touch.pointerId = pointerId;
		ev.touch.pressed = pressed;

		pushEvent(ev);
	}

	void pushJoypadEvent(uint8_t i, bool connected)
	{
		OsEvent ev;
		ev.type = OsEvent::JOYPAD;
		ev.joypad.type = OsJoypadEvent::CONNECTED;
		ev.joypad.index = i;
		ev.joypad.connected = connected;

		pushEvent(ev);
	}

	void pushJoypadEvent(uint8_t i, uint8_t button, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::JOYPAD;
		ev.joypad.type = OsJoypadEvent::BUTTON;
		ev.joypad.index = i;
		ev.joypad.button = button;
		ev.joypad.pressed = pressed;

		pushEvent(ev);
	}

	void pushJoypadEvent(uint8_t i, uint8_t axis, float x, float y, float z)
	{
		OsEvent ev;
		ev.type = OsEvent::JOYPAD;
		ev.joypad.type = OsJoypadEvent::AXIS;
		ev.joypad.index = i;
		ev.joypad.button = axis;
		ev.joypad.x = x;
		ev.joypad.y = y;
		ev.joypad.z = z;

		pushEvent(ev);
	}

	void pushExitEvent(int32_t code)
	{
		OsEvent ev;
		ev.type = OsEvent::EXIT;
		ev.exit.code = code;

		pushEvent(ev);
	}

	void pushPauseEvent()
	{
		OsEvent ev;
		ev.type = OsEvent::PAUSE;

		pushEvent(ev);
	}

	void pushResumeEvent()
	{
		OsEvent ev;
		ev.type = OsEvent::RESUME;

		pushEvent(ev);
	}

	void pushMetricsEvent(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		OsEvent ev;
		ev.type = OsEvent::METRICS;
		ev.metrics.x = x;
		ev.metrics.y = y;
		ev.metrics.width = width;
		ev.metrics.height = height;

		pushEvent(ev);
	}

	void pushNoneEvent()
	{
		OsEvent ev;
		ev.type = OsEvent::NONE;

		pushEvent(ev);
	}

	bool pushEvent(const OsEvent& ev)
	{
		int currentTail = this->tail.load();
		int nextTail = increment(currentTail);
		if (nextTail != this->head.load())
		{
			this->queue[currentTail] = ev;
			this->tail.store(nextTail);
			return true;
		}

		return false;
	}

	bool popEvent(OsEvent& ev)
	{
		const int currentHead = this->head.load();
		if (currentHead == this->tail.load())
		{
			return false;
		}

		ev = this->queue[currentHead];
		this->head.store(increment(currentHead));
		return true;
	}

	int increment(int idx) const
	{
	  return (idx + 1) % MAX_OS_EVENTS;
	}

private:
	OsEvent queue[MAX_OS_EVENTS];
	AtomicInt tail = 0;
	AtomicInt head = 0;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka