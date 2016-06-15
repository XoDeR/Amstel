// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/FileSystem/File.h"

namespace Rio
{

class BinaryWriter
{
public:
	BinaryWriter(File& file)
		: file(file)
	{
	}

	void write(const void* data, uint32_t size)
	{
		file.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		file.write(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
	{
		file.skip(bytes);
	}
private:
	File& file;
};

class BinaryReader
{
public:
	BinaryReader(File& file)
		: file(file)
	{
	}

	void read(void* data, uint32_t size)
	{
		file.read(data, size);
	}

	template <typename T>
	void read(T& data)
	{
		file.read(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
	{
		file.skip(bytes);
	}
public:
	File& file;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka