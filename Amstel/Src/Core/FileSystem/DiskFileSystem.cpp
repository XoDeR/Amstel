// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/FileSystem/DiskFileSystem.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/File.h"
#include "Core/Base/Os.h"
#include "Core/FileSystem/Path.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"

#if RIO_PLATFORM_POSIX
	#include <stdio.h>
	#include <errno.h>
#elif RIO_PLATFORM_WINDOWS
	#include "tchar.h"
	#include "Device/Windows/Headers_Windows.h"
#endif // RIO_PLATFORM_

namespace Rio
{

class DiskFile : public File
{
#if RIO_PLATFORM_POSIX
	FILE* file = nullptr;
#elif RIO_PLATFORM_WINDOWS
	HANDLE file = INVALID_HANDLE_VALUE;
	bool isEndOfFile = false;
#endif // RIO_PLATFORM_

public:
	DiskFile()
	{
	}

	virtual ~DiskFile()
	{
		close();
	}

	// Opens the file located at <path> with the given <mode>
	void open(const char* path, FileOpenMode::Enum mode)
	{
#if RIO_PLATFORM_POSIX
		file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
		RIO_ASSERT(file != nullptr, "fopen: errno = %d, path = '%s'", errno, path);
#elif RIO_PLATFORM_WINDOWS
		file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, 0
			, NULL
			, OPEN_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		RIO_ASSERT(file != INVALID_HANDLE_VALUE
			, "CreateFile: GetLastError = %d, path = '%s'"
			, GetLastError()
			, path
			);
#endif // RIO_PLATFORM_
	}

	void close()
	{
		if (getIsOpen())
		{
#if RIO_PLATFORM_POSIX
			fclose(file);
			file = nullptr;
#elif RIO_PLATFORM_WINDOWS
			CloseHandle(file);
			file = INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_
		}
	}

	bool getIsOpen() const
	{
#if RIO_PLATFORM_POSIX
		return file != NULL;
#elif RIO_PLATFORM_WINDOWS
		return file != INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_
	}

	uint32_t getSize()
	{
#if RIO_PLATFORM_POSIX
		long position = ftell(file);
		RIO_ASSERT(position != -1, "ftell: errno = %d", errno);
		int err = fseek(file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		long size = ftell(file);
		RIO_ASSERT(size != -1, "ftell: errno = %d", errno);
		err = fseek(file, (long)position, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		RIO_UNUSED(err);
		return (uint32_t)size;
#elif RIO_PLATFORM_WINDOWS
		return GetFileSize(file, NULL);
#endif // RIO_PLATFORM_
	}

	uint32_t getPosition()
	{
#if RIO_PLATFORM_POSIX
		long position = ftell(file);
		RIO_ASSERT(position != -1, "ftell: errno = %d", errno);
		return (uint32_t)position;
#elif RIO_PLATFORM_WINDOWS
		DWORD position = SetFilePointer(file, 0, NULL, FILE_CURRENT);
		RIO_ASSERT(position != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
		return (uint32_t)position;
#endif // RIO_PLATFORM_
	}

	bool getIsEndOfFile()
	{
#if RIO_PLATFORM_POSIX
		return feof(file) != 0;
#elif RIO_PLATFORM_WINDOWS
		return isEndOfFile;
#endif // RIO_PLATFORM_
	}

	void seek(uint32_t position)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(file, (long)position, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(file, position, NULL, FILE_BEGIN);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif // RIO_PLATFORM_
		RIO_UNUSED(err);
	}

	void seekToEnd()
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(file, 0, NULL, FILE_END);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif // RIO_PLATFORM_
		RIO_UNUSED(err);
	}

	void skip(uint32_t bytes)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(file, bytes, SEEK_CUR);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(file, bytes, NULL, FILE_CURRENT);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif // RIO_PLATFORM_
		RIO_UNUSED(err);
	}

	uint32_t read(void* data, uint32_t size)
	{
		RIO_ASSERT(data != NULL, "Data must be != NULL");
#if RIO_PLATFORM_POSIX
		size_t bytesRead = fread(data, 1, size, file);
		RIO_ASSERT(ferror(file) == 0, "fread error");
		return (uint32_t)bytesRead;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytesRead;
		BOOL err = ReadFile(file, data, size, &bytesRead, NULL);
		RIO_ASSERT(err == TRUE, "ReadFile: GetLastError = %d", GetLastError());
		isEndOfFile = err && bytesRead == 0;
		return bytesRead;
#endif // RIO_PLATFORM_
	}

	uint32_t write(const void* data, uint32_t size)
	{
		RIO_ASSERT(data != NULL, "Data must be != NULL");
#if RIO_PLATFORM_POSIX
		size_t bytes_written = fwrite(data, 1, size, file);
		RIO_ASSERT(ferror(file) == 0, "fwrite error");
		return (uint32_t)bytes_written;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(file, data, size, &bytes_written, NULL);
		RIO_ASSERT(size == bytes_written
			, "WriteFile: GetLastError = %d"
			, GetLastError()
			);
		return bytes_written;
#endif // RIO_PLATFORM_
	}

	void flush()
	{
#if RIO_PLATFORM_POSIX
		int err = fflush(file);
		RIO_ASSERT(err == 0, "fflush: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = FlushFileBuffers(file);
		RIO_ASSERT(err != 0
			, "FlushFileBuffers: GetLastError = %d"
			, GetLastError()
			);
#endif // RIO_PLATFORM_
		RIO_UNUSED(err);
	}
};

DiskFileSystem::DiskFileSystem(Allocator& a)
	: allocator(&a)
	, prefix(a)
{
}

void DiskFileSystem::setPrefix(const char* prefix)
{
	this->prefix.set(prefix, getStringLength32(prefix));
}

File* DiskFileSystem::open(const char* path, FileOpenMode::Enum mode)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	DiskFile* file = RIO_NEW(*allocator, DiskFile)();
	file->open(absolutePath.getCStr(), mode);
	return file;
}

void DiskFileSystem::close(File& file)
{
	RIO_DELETE(*allocator, &file);
}

bool DiskFileSystem::getDoesExist(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	return OsFn::getDoesExist(absolutePath.getCStr());
}

bool DiskFileSystem::getIsDirectory(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	return OsFn::getIsDirectory(absolutePath.getCStr());
}

bool DiskFileSystem::getIsFile(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	return OsFn::getIsFile(absolutePath.getCStr());
}

uint64_t DiskFileSystem::getLastModifiedTime(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	return OsFn::getLastModifiedTime(absolutePath.getCStr());
}

void DiskFileSystem::createDirectory(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	if (!OsFn::getDoesExist(absolutePath.getCStr()))
	{
		OsFn::createDirectory(absolutePath.getCStr());
	}
}

void DiskFileSystem::deleteDirectory(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::deleteDirectory(absolutePath.getCStr());
}

void DiskFileSystem::createFile(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::createFile(absolutePath.getCStr());
}

void DiskFileSystem::deleteFile(const char* path)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::deleteFile(absolutePath.getCStr());
}

void DiskFileSystem::getFileList(const char* path, Vector<DynamicString>& files)
{
	RIO_ASSERT_NOT_NULL(path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::getFileList(absolutePath.getCStr(), files);
}

void DiskFileSystem::getAbsolutePath(const char* path, DynamicString& osPath)
{
	if (PathFn::getIsAbsolute(path))
	{
		osPath = path;
		return;
	}

	PathFn::join(prefix.getCStr(), path, osPath);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka