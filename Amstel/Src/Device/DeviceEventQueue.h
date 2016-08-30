// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Thread/AtomicInt.h"

namespace Rio
{

struct OsEventType
{
	enum Enum
	{
		BUTTON,
		AXIS,
		STATUS,
		RESOLUTION,
		EXIT,
		PAUSE,
		RESUME,
		NONE
	};
};

struct ButtonEvent
{
	uint16_t type;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t buttonIndex : 8;
	uint16_t pressed : 1;
};

struct AxisEvent
{
	uint16_t type;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t axisIndex : 4;
	float axisX;
	float axisY;
	float axisZ;
};

struct StatusEvent
{
	uint16_t type;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t connected : 1;
};

struct ResolutionEvent
{
	uint16_t type;
	uint16_t width;
	uint16_t height;
};

union OsEvent
{
	uint16_t type;
	ButtonEvent button;
	AxisEvent axis;
	StatusEvent status;
	ResolutionEvent resolution;
};

#define MAX_OS_EVENTS 4096

// Single Producer Single Consumer event queue
// Used only to pass events from os thread to main thread
struct DeviceEventQueue
{
	void pushButtonEvent(uint16_t deviceId, uint16_t deviceIndex, uint16_t buttonIndex, bool pressed)
	{
		OsEvent ev;
		ev.button.type = OsEventType::BUTTON;
		ev.button.deviceId = deviceId;
		ev.button.deviceIndex = deviceIndex;
		ev.button.buttonIndex = buttonIndex;
		ev.button.pressed = pressed;

		pushEvent(ev);
	}

	void pushAxisEvent(uint16_t deviceId, uint16_t deviceIndex, uint16_t axisIndex, float axisX, float axisY, float axisZ)
	{
		OsEvent ev;
		ev.axis.type = OsEventType::AXIS;
		ev.axis.deviceId = deviceId;
		ev.axis.deviceIndex = deviceIndex;
		ev.axis.axisIndex = axisIndex;
		ev.axis.axisX = axisX;
		ev.axis.axisY = axisY;
		ev.axis.axisZ = axisZ;

		pushEvent(ev);
	}

	void pushStatusEvent(uint16_t deviceId, uint16_t deviceIndex, bool connected)
	{
		OsEvent ev;
		ev.status.type = OsEventType::STATUS;
		ev.status.deviceId = deviceId;
		ev.status.deviceIndex = deviceIndex;
		ev.status.connected = connected;

		pushEvent(ev);
	}

	void pushResolutionEvent(uint16_t width, uint16_t height)
	{
		OsEvent ev;
		ev.resolution.type = OsEventType::RESOLUTION;
		ev.resolution.width = width;
		ev.resolution.height = height;

		pushEvent(ev);
	}

	void pushExitEvent()
	{
		OsEvent ev;
		ev.type = OsEventType::EXIT;

		pushEvent(ev);
	}

	void pushPauseEvent()
	{
		OsEvent ev;
		ev.type = OsEventType::PAUSE;

		pushEvent(ev);
	}

	void pushResumeEvent()
	{
		OsEvent ev;
		ev.type = OsEventType::RESUME;

		pushEvent(ev);
	}

	void pushNoneEvent()
	{
		OsEvent ev;
		ev.type = OsEventType::NONE;

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