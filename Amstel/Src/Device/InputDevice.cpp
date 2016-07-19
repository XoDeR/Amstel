// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/InputDevice.h"
#include "Core/Error/Error.h"
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
	RIO_ASSERT(id < buttonListCount, "Index out of bounds");
	return (~lastStateList[id] & stateList[id]) != 0;
}

bool InputDevice::getIsReleased(uint8_t id) const
{
	RIO_ASSERT(id < buttonListCount, "Index out of bounds");
	return (lastStateList[id] & ~stateList[id]) != 0;
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
	RIO_ASSERT(id < axisListCount, "Index out of bounds");
	return axisList[id];
}

const char* InputDevice::getButtonName(uint8_t id)
{
	RIO_ASSERT(id < buttonListCount, "Index out of bounds");
	return buttonNameList[id];
}

const char* InputDevice::getAxisName(uint8_t id)
{
	RIO_ASSERT(id < axisListCount, "Index out of bounds");
	return axisNameList[id];
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

	RIO_ASSERT(false, "Unknown button name");
	return 0;
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

	RIO_ASSERT(false, "Unknown axis name");
	return 0;
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
	InputDevice* create(Allocator& a, const char* name, uint8_t buttonListCount, uint8_t axesCount, const char** buttonNameList, const char** axisNameList)
	{
		const uint32_t size = 0
			+ sizeof(InputDevice)
			+ sizeof(uint8_t)*buttonListCount *2
			+ sizeof(Vector3)*axesCount
			+ sizeof(char*)*buttonListCount
			+ sizeof(char*)*axesCount
			+ sizeof(StringId32)*buttonListCount
			+ sizeof(StringId32)*axesCount
			+ getStringLength32(name) + 1
			;

		InputDevice* inputDevice = (InputDevice*)a.allocate(size);

		inputDevice->isConnected = false;
		inputDevice->buttonListCount = buttonListCount;
		inputDevice->axisListCount = axesCount;
		inputDevice->lastButton = 0;

		inputDevice->lastStateList = (uint8_t*)&inputDevice[1];
		inputDevice->stateList = (uint8_t*)(inputDevice->lastStateList + buttonListCount);
		inputDevice->axisList = (Vector3*)(inputDevice->stateList + buttonListCount);
		inputDevice->buttonNameList = (const char**)(inputDevice->axisList + axesCount);
		inputDevice->axisNameList = (const char**)(inputDevice->buttonNameList + buttonListCount);
		inputDevice->buttonNameHashList = (StringId32*)(inputDevice->axisNameList + axesCount);
		inputDevice->axisNameHashList = (StringId32*)(inputDevice->buttonNameHashList + buttonListCount);
		inputDevice->inputDeviceName = (char*)(inputDevice->axisNameHashList + axesCount);

		memset(inputDevice->lastStateList, 0, sizeof(uint8_t)*buttonListCount);
		memset(inputDevice->stateList, 0, sizeof(uint8_t)*buttonListCount);
		memset(inputDevice->axisList, 0, sizeof(Vector3)*axesCount);
		memcpy(inputDevice->buttonNameList, buttonNameList, sizeof(const char*)*buttonListCount);
		memcpy(inputDevice->axisNameList, axisNameList, sizeof(const char*)*axesCount);

		for (uint32_t i = 0; i < buttonListCount; ++i)
		{
			inputDevice->buttonNameHashList[i] = StringId32(buttonNameList[i]);
		}

		for (uint32_t i = 0; i < axesCount; ++i)
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