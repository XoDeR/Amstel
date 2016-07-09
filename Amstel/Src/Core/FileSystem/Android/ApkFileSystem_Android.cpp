// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_ANDROID

#include "Core/FileSystem/Android/ApkFileSystem_Android.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/File.h"
#include "Core/Base/Os.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"

namespace Rio
{

class ApkFile : public File
{
public:
	ApkFile(AAssetManager* assetManager)
		: assetManager(assetManager)
	{
	}

	virtual ~ApkFile()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum /*mode*/)
	{
		asset = AAssetManager_open(assetManager, path, AASSET_MODE_RANDOM);
		RIO_ASSERT(asset != nullptr, "AAssetManager_open: failed to open %s", path);
	}

	void close()
	{
		if (asset != nullptr)
		{
			AAsset_close(asset);
			asset = nullptr;
		}
	}

	uint32_t getSize()
	{
		return AAsset_getLength(asset);
	}

	uint32_t getPosition()
	{
		return uint32_t(AAsset_getLength(asset) - AAsset_getRemainingLength(asset));
	}

	bool getIsEndOfFile()
	{
		return AAsset_getRemainingLength(asset) == 0;
	}

	void seek(uint32_t position)
	{
		off_t seekResult = AAsset_seek(asset, (off_t)position, SEEK_SET);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	void seekToEnd()
	{
		off_t seekResult = AAsset_seek(asset, 0, SEEK_END);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	void skip(uint32_t bytes)
	{
		off_t seekResult = AAsset_seek(asset, (off_t)bytes, SEEK_CUR);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	uint32_t read(void* data, uint32_t size)
	{
		RIO_ASSERT_NOT_NULL(data);
		return (uint32_t)AAsset_read(asset, data, size);
	}

	uint32_t write(const void* /*data*/, uint32_t /*size*/)
	{
		RIO_ASSERT(false, "Apk files are read only!");
		return 0;
	}

	void flush()
	{
		// Not needed
	}
private:
	AAssetManager* assetManager;
	AAsset* asset = nullptr;
};

ApkFileSystem::ApkFileSystem(Allocator& a, AAssetManager* assetManager)
	: allocator(&a)
	, assetManager(assetManager)
{
}

File* ApkFileSystem::open(const char* path, FileOpenMode::Enum mode)
{
	RIO_ASSERT_NOT_NULL(path);
	RIO_ASSERT(mode == FileOpenMode::READ, "Cannot open for writing in Android assets folder");
	ApkFile* file = RIO_NEW(*allocator, ApkFile)(assetManager);
	file->open(path, mode);
	return file;
}

void ApkFileSystem::close(File& file)
{
	RIO_DELETE(*allocator, &file);
}

bool ApkFileSystem::getDoesExist(const char* path)
{
	return false;
}

bool ApkFileSystem::getIsDirectory(const char* path)
{
	return true;
}

bool ApkFileSystem::getIsFile(const char* path)
{
	return true;
}

uint64_t ApkFileSystem::getLastModifiedTime(const char* path)
{
	return 0;
}

void ApkFileSystem::createDirectory(const char* /*path*/)
{
	RIO_ASSERT(false, "Cannot create directory in Android assets folder");
}

void ApkFileSystem::deleteDirectory(const char* /*path*/)
{
	RIO_ASSERT(false, "Cannot delete directory in Android assets folder");
}

void ApkFileSystem::createFile(const char* /*path*/)
{
	RIO_ASSERT(false, "Cannot create file in Android assets folder");
}

void ApkFileSystem::deleteFile(const char* /*path*/)
{
	RIO_ASSERT(false, "Cannot delete file in Android assets folder");
}

void ApkFileSystem::getFileList(const char* path, Vector<DynamicString>& files)
{
	RIO_ASSERT_NOT_NULL(path);

	AAssetDir* rootDirectory = AAssetManager_openDir(assetManager, path);
	RIO_ASSERT(rootDirectory != nullptr, "Failed to open Android assets folder");

	const char* fileName = nullptr;
	while ((fileName = AAssetDir_getNextFileName(rootDirectory)) != nullptr)
	{
		TempAllocator512 ta;
		DynamicString name(fileName, ta);
		VectorFn::pushBack(files, name);
	}

	AAssetDir_close(rootDirectory);
}

void ApkFileSystem::getAbsolutePath(const char* path, DynamicString& osPath)
{
	osPath = path;
}

} // namespace Rio

#endif // RIO_PLATFORM_ANDROID
// Copyright (c) 2016 Volodymyr Syvochka