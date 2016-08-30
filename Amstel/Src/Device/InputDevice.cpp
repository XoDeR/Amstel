// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/InputDevice.h"

#include "Core/Error/Error.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/Allocator.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Strings/StringId.h"

#include <string.h> // strcpy, memset

namespace Rio
{

const char* InputDevice::getName() const
{
	return inputDeviceName;
}

bool InputDevice::getIsConnected() const
{
	return isConnected;
}

uint8_t InputDevice::getButtonsCount() const
{
	return buttonListCount;
}

uint8_t InputDevice::getAxesCount() const
{
	return axisListCount;
}

bool InputDevice::getIsPressed(uint8_t id) const
{
	return id < buttonListCount
		? (~lastStateList[id] & stateList[id]) != 0
		: false
		;
}

bool InputDevice::getIsReleased(uint8_t id) const
{
	return id < buttonListCount
		? (lastStateList[id] & ~stateList[id]) != 0
		: false
		;
}

bool InputDevice::getIsAnyPressed() const
{
	return getIsPressed(lastButton);
}

bool InputDevice::getIsAnyReleased() const
{
	return getIsReleased(lastButton);
}

Vector3 InputDevice::getAxis(uint8_t id) const
{
	return id < axisListCount
		? axisList[id]
		: VECTOR3_ZERO
		;
}

const char* InputDevice::getButtonName(uint8_t id)
{
	return id < buttonListCount
		? buttonNameList[id]
		: nullptr
		;
}

const char* InputDevice::getAxisName(uint8_t id)
{
	return id < axisListCount
		? axisNameList[id]
		: nullptr
		;
}

uint8_t InputDevice::getButtonId(StringId32 name)
{
	for (uint32_t i = 0; i < buttonListCount; ++i)
	{
		if (buttonNameHashList[i] == name)
		{
			return i;
		}
	}
	return UINT8_MAX;
}

uint8_t InputDevice::getAxisId(StringId32 name)
{
	for (uint32_t i = 0; i < axisListCount; ++i)
	{
		if (axisNameHashList[i] == name)
		{
			return i;
		}
	}
	return UINT8_MAX;
}

void InputDevice::setIsConnected(bool connected)
{
	this->isConnected = connected;
}

void InputDevice::setButtonState(uint8_t i, bool state)
{
	RIO_ASSERT(i < buttonListCount, "Index out of bounds");
	lastButton = i;
	stateList[i] = state;
}

void InputDevice::setAxis(uint8_t i, const Vector3& value)
{
	RIO_ASSERT(i < axisListCount, "Index out of bounds");
	axisList[i] = value;
}

void InputDevice::update()
{
	memcpy(lastStateList, stateList, sizeof(uint8_t)*buttonListCount);
}

namespace InputDeviceFn
{
	InputDevice* create(Allocator& a, const char* name, uint8_t buttonListCount, uint8_t axisListCount, const char** buttonNameList, const char** axisNameList)
	{
		const uint32_t size = 0
			+ sizeof(InputDevice) + alignof(InputDevice)
			+ sizeof(uint8_t)*buttonListCount * 2 + alignof(uint8_t)
			+ sizeof(Vector3)*axisListCount + alignof(Vector3)
			+ sizeof(char*)*buttonListCount + alignof(char*)
			+ sizeof(char*)*axisListCount + alignof(char*)
			+ sizeof(StringId32)*buttonListCount + alignof(StringId32)
			+ sizeof(StringId32)*axisListCount + alignof(StringId32)
			+ getStringLength32(name) + 1 + alignof(char)
			;

		InputDevice* inputDevice = (InputDevice*)a.allocate(size);

		inputDevice->isConnected = false;
		inputDevice->buttonListCount = buttonListCount;
		inputDevice->axisListCount = axisListCount;
		inputDevice->lastButton = 0;

		inputDevice->lastStateList = (uint8_t*)&inputDevice[1];
		inputDevice->stateList = (uint8_t*)MemoryFn::alignTop(inputDevice->lastStateList + buttonListCount, alignof(uint8_t));
		inputDevice->axisList = (Vector3*)MemoryFn::alignTop(inputDevice->stateList + buttonListCount, alignof(Vector3));
		inputDevice->buttonNameList = (const char**)MemoryFn::alignTop(inputDevice->axisList + axisListCount, alignof(const char*));
		inputDevice->axisNameList = (const char**)MemoryFn::alignTop(inputDevice->buttonNameList + buttonListCount, alignof(const char*));
		inputDevice->buttonNameHashList = (StringId32*)MemoryFn::alignTop(inputDevice->axisNameList + axisListCount, alignof(StringId32));
		inputDevice->axisNameHashList = (StringId32*)MemoryFn::alignTop(inputDevice->buttonNameHashList + buttonListCount, alignof(StringId32));
		inputDevice->inputDeviceName = (char*)MemoryFn::alignTop(inputDevice->axisNameHashList + axisListCount, alignof(char));

		memset(inputDevice->lastStateList, 0, sizeof(uint8_t)*buttonListCount);
		memset(inputDevice->stateList, 0, sizeof(uint8_t)*buttonListCount);
		memset(inputDevice->axisList, 0, sizeof(Vector3)*axisListCount);
		memcpy(inputDevice->buttonNameList, buttonNameList, sizeof(const char*)*buttonListCount);
		memcpy(inputDevice->axisNameList, axisNameList, sizeof(const char*)*axisListCount);

		for (uint32_t i = 0; i < buttonListCount; ++i)
		{
			inputDevice->buttonNameHashList[i] = StringId32(buttonNameList[i]);
		}

		for (uint32_t i = 0; i < axisListCount; ++i)
		{
			inputDevice->axisNameHashList[i] = StringId32(axisNameList[i]);
		}

		strcpy(inputDevice->inputDeviceName, name);

		return inputDevice;
	}

	void destroy(Allocator& a, InputDevice& inputDevice)
	{
		a.deallocate(&inputDevice);
	}
} // namespace InputDeviceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka