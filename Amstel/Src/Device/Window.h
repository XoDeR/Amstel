// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/MemoryTypes.h"
#include "Core/Base/Types.h"

namespace Rio
{

struct Window
{
	virtual void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t parent) = 0;
	virtual void close() = 0;
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void resize(uint16_t width, uint16_t height) = 0;
	// Moves the window to [<x>,<y>]
	virtual void move(uint16_t x, uint16_t y) = 0;
	virtual void minimize() = 0;
	virtual void restore() = 0;
	virtual const char* getTitle() = 0;
	virtual void setTitle (const char* title) = 0;
	// Returns the native window handle
	virtual void* getHandle() = 0;
	// Sets whether to show the cursor
	virtual void setShowCursor(bool show) = 0;
	virtual void setupBgfx() = 0;
};

namespace WindowFn
{
	// Creates a new window.
	Window* create(Allocator& a);
	// Destroys the window @a w.
	void destroy(Allocator& a, Window& w);
} // namespace WindowFn

} // namespace Rio
  // Copyright (c) 2016 Volodymyr Syvochka