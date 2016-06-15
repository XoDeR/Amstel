// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/FileSystem/File.h"

namespace Rio
{

// Bit bucket file
// Discards all data written to it and provides null data reading from it
class NullFile: public File
{
public:
	void open(const char* /*path*/, FileOpenMode::Enum /*mode*/) {}
	void close() {}
	// Returns always 0xFFFFFFFF
	uint32_t getSize() { return ~0; }
	// Returns always zero
	uint32_t getPosition() { return 0; }
	// Returns always false
	bool getIsEndOfFile() { return false; }
	void seek(uint32_t position) { (void)position; }
	void seekToEnd() {}
	void skip(uint32_t bytes) { (void)bytes; }
	// Fills buffer with zeroes
	uint32_t read(void* data, uint32_t size)
	{
		for (uint32_t i = 0; i < size; ++i)
		{
			((uint8_t*)data)[i] = 0;
		}
		return size;
	}
	uint32_t write(const void* /*data*/, uint32_t size)
	{
		return size;
	}
	void flush() {};
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka