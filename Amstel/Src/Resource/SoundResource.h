// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringId.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

struct SoundType
{
	enum Enum
	{
		WAV,
		OGG
	};
};

struct SoundResource
{
	uint32_t version;
	uint32_t size;
	uint32_t sampleRate;
	uint32_t averageBytesPerSample;
	uint32_t channels;
	uint16_t blockSize;
	uint16_t bitsPerSample;
	uint32_t soundType;
};

namespace SoundResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace SoundResourceInternalFn

namespace SoundResourceFn
{
	const char* getData(const SoundResource* soundResource);
} // namespace SoundResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka