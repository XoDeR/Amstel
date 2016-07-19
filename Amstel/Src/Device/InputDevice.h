// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Math/MathTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Strings/StringId.h"

namespace Rio
{

struct InputDevice
{
	// Returns the name of the input device
	const char* getName() const;
	// Returns whether the input device is connected and functioning
	bool getIsConnected() const;
	// Returns the number of buttons of the input device
	uint8_t getButtonsCount() const;
	// Returns the number of axes of the input devices
	uint8_t getAxesCount() const;
	// Returns whether the button <id> is pressed in the current frame
	bool getIsPressed(uint8_t id) const;
	// Returns whether the button <id> is released in the current frame
	bool getIsReleased(uint8_t id) const;
	// Returns whether any button is pressed in the current frame
	bool getIsAnyPressed() const;
	// Returns whether any button is released in the current frame
	bool getIsAnyReleased() const;
	// Returns the value of the axis <id>
	Vector3 getAxis(uint8_t id) const;
	// Returns the name of the button <id>
	const char* getButtonName(uint8_t id);
	// Returns the name of the axis <id>
	const char* getAxisName(uint8_t id);
	// Returns the id of the button <name>
	uint8_t getButtonId(StringId32 name);
	// Returns the id of the axis <name>
	uint8_t getAxisId(StringId32 name);
	void setIsConnected(bool connected);
	void setButtonState(uint8_t i, bool state);
	void setAxis(uint8_t i, const Vector3& value);
	void update();

	bool isConnected = false;
	uint8_t buttonListCount;
	uint8_t axisListCount;
	uint8_t lastButton;

	uint8_t* lastStateList; // buttonListCount
	uint8_t* stateList;  // buttonListCount
	Vector3* axisList; // axesCount
	const char** buttonNameList; // buttonListCount
	const char** axisNameList;   // axesCount
	StringId32* buttonNameHashList;  // buttonListCount
	StringId32* axisNameHashList;    // axesCount
	char* inputDeviceName; // getStringLength32(name) + 1
};

namespace InputDeviceFn
{
	InputDevice* create(Allocator& a, const char* name, uint8_t buttonListCount, uint8_t axesCount, const char** buttonNameList, const char** axisNameList);
	void destroy(Allocator& a, InputDevice& id);
} // namespace InputDeviceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka