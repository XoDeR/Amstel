// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"
#include "Core/Base/Types.h"
#include "Device/InputTypes.h"
#include "Core/Memory/MemoryTypes.h"

namespace Rio
{

class InputManager
{
public:
	InputManager(Allocator& a);
	~InputManager();
	InputDevice* getKeyboard();
	InputDevice* getMouse();
	InputDevice* getTouch();
	uint8_t getJoypadListCount();
	InputDevice* getJoypad(uint8_t i);
	void update();
private:
	Allocator* allocator;
	InputDevice* keyboard = nullptr;
	InputDevice* mouse = nullptr;
	InputDevice* touch = nullptr;
	InputDevice* joypadList[RIO_MAX_JOYPADS];
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka