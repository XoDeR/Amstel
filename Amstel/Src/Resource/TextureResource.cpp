// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/TextureResource.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonR.h"
#include "Core/Base/Os.h"
#include "Core/Strings/StringStream.h"
#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#if RIO_DEVELOPMENT
	#define TEXTUREC_NAME "texturec-development-"
#elif RIO_DEBUG
	#define TEXTUREC_NAME "texturec-debug-"
#else
	#define TEXTUREC_NAME "texturec-release-"
#endif  // RIO_DEBUG
#if RIO_ARCH_32BIT
	#define TEXTUREC_BITS "32"
#elif RIO_ARCH_64BIT
	#define TEXTUREC_BITS "64"
#endif // RIO_ARCH_32BIT
#if RIO_PLATFORM_LINUX
	#define TEXTUREC_PATH "./" TEXTUREC_NAME "" TEXTUREC_BITS
#elif RIO_PLATFORM_WINDOWS
	#define TEXTUREC_PATH TEXTUREC_NAME "" TEXTUREC_BITS ".exe"
#else
	#define TEXTUREC_PATH ""
#endif // RIO_PLATFORM_LINUX

namespace Rio
{

namespace TextureResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(buffer, jsonObject);

		DynamicString name(ta);
		JsonRFn::parseString(jsonObject["source"], name);
		RESOURCE_COMPILER_ASSERT_FILE_EXISTS(name.getCStr(), compileOptions);

		const bool generateMips = JsonRFn::parseBool(jsonObject["generateMips"]);
		const bool isNormalMap  = JsonRFn::parseBool(jsonObject["isNormalMap"]);

		DynamicString textureSourceName(ta);
		DynamicString textureOutputName(ta);
		compileOptions.getAbsolutePath(name.getCStr(), textureSourceName);
		compileOptions.getTemporaryPath("texture.ktx", textureOutputName);

		StringStream args(ta);
		args << " -f " << textureSourceName.getCStr();
		args << " -o " << textureOutputName.getCStr();
		args << (generateMips ? " -m " : "");
		args << (isNormalMap ? " -n " : "");

		StringStream output(ta);
		int exitCode = OsFn::executeProcess(TEXTUREC_PATH, StringStreamFn::getCStr(args), output);
		RESOURCE_COMPILER_ASSERT(exitCode == 0
			, compileOptions
			, "Failed to compile texture:\n%s"
			, StringStreamFn::getCStr(output)
			);

		Buffer blob = compileOptions.readTemporary(textureOutputName.getCStr());
		compileOptions.deleteFile(textureOutputName.getCStr());

		// Write DDS
		compileOptions.write(RESOURCE_VERSION_TEXTURE);
		compileOptions.write(ArrayFn::getCount(blob));
		compileOptions.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_TEXTURE, "Wrong version");

		uint32_t size;
		binaryReader.read(size);

		TextureResource* textureResource = (TextureResource*)a.allocate(sizeof(TextureResource) + size);

		void* data = &textureResource[1];
		binaryReader.read(data, size);

		textureResource->memoryBuffer = bgfx::makeRef(data, size);
		textureResource->handle.idx = bgfx::invalidHandle;

		return textureResource;
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		TextureResource* textureResource = (TextureResource*)resourceManager.get(RESOURCE_TYPE_TEXTURE, id);
		textureResource->handle = bgfx::createTexture(textureResource->memoryBuffer);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		TextureResource* textureResource = (TextureResource*)resourceManager.get(RESOURCE_TYPE_TEXTURE, id);
		bgfx::destroyTexture(textureResource->handle);
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace TextureResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka