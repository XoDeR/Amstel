// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Base/Types.h"

namespace Rio
{

class File
{
public:
	File() {}
	virtual ~File() {}

	virtual void open(const char* path, FileOpenMode::Enum mode) = 0;
	virtual void close() = 0;
	// Returns the size of file in bytes
	virtual uint32_t getSize() = 0;
	// Returns the number of bytes from the beginning of the file to the cursor position
	virtual uint32_t getPosition() = 0;
	// Returns whether the cursor position is at end of file
	virtual bool getIsEndOfFile() = 0;
	// Sets the cursor to the position.
	virtual void seek(uint32_t position) = 0;
	// Sets the cursor position to the end of the file
	virtual void seekToEnd() = 0;
	// Sets the cursor position to <bytes> after current position
	virtual void skip(uint32_t bytes) = 0;
	// Reads <size> bytes from file
	virtual uint32_t read(void* data, uint32_t size) = 0;
	// Writes <size> bytes to file
	virtual uint32_t write(const void* data, uint32_t size) = 0;
	// Forces the previous write operations to be completed
	virtual void flush() = 0;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka