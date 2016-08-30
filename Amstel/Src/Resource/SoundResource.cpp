// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/SoundResource.h"

#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Resource/CompileOptions.h"

namespace Rio
{

namespace SoundResourceInternalFn
{
	struct WavHeader
	{
		char riff[4]; // Should contain RIFF
		int32_t chunkSize; // Not Needed
		char wave[4]; // Should contain WAVE
		char formatList[4]; // Should contain formatList
		int32_t formatSize; // Size of format chunk
		int16_t formatTag; // Identifies way data is stored, 1 means no compression
		int16_t formatChannels; // Channel, 1 -- mono, 2 -- stereo
		int32_t formatSampleRate; // Samples per second
		int32_t formatAverageBytesPerSample; // Average bytes per sample
		int16_t formatBlockAlign; // Block alignment
		int16_t formatBitsPerSample; // Number of bits per sample
		char data[4]; // Should contain data
		int32_t dataSize; // Data dimension
	};

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		DynamicString name(ta);
		JsonRFn::parseString(jsonObject["source"], name);

		Buffer sound = compileOptions.read(name.getCStr());
		const WavHeader* wavHeader = (const WavHeader*)ArrayFn::begin(sound);
		const char* wavData = (const char*)&wavHeader[1];

		// Write
		SoundResource soundResource;
		soundResource.version = RESOURCE_VERSION_SOUND;
		soundResource.size = wavHeader->dataSize;
		soundResource.sampleRate = wavHeader->formatSampleRate;
		soundResource.averageBytesPerSample = wavHeader->formatAverageBytesPerSample;
		soundResource.channels = wavHeader->formatChannels;
		soundResource.blockSize = wavHeader->formatBlockAlign;
		soundResource.bitsPerSample = wavHeader->formatBitsPerSample;
		soundResource.soundType = SoundType::WAV;

		compileOptions.write(soundResource.version);
		compileOptions.write(soundResource.size);
		compileOptions.write(soundResource.sampleRate);
		compileOptions.write(soundResource.averageBytesPerSample);
		compileOptions.write(soundResource.channels);
		compileOptions.write(soundResource.blockSize);
		compileOptions.write(soundResource.bitsPerSample);
		compileOptions.write(soundResource.soundType);

		compileOptions.write(wavData, wavHeader->dataSize);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t fileSize = file.getSize();
		void* resource = a.allocate(fileSize);
		file.read(resource, fileSize);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_SOUND, "Wrong version");
		return resource;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}
} // namespace SoundResourceInternalFn

namespace SoundResourceFn
{
	const char* getData(const SoundResource* soundResource)
	{
		return (char*)&soundResource[1];
	}
} // namespace SoundResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka