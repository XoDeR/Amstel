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
#endif

namespace Rio
{
class DiskFile : public File
{
#if RIO_PLATFORM_POSIX
	FILE* _file;
#elif RIO_PLATFORM_WINDOWS
	HANDLE _file;
	bool _eof;
#endif

public:

	/// Opens the file located at @a path with the given @a mode.
	DiskFile()
#if RIO_PLATFORM_POSIX
		: _file(NULL)
#elif RIO_PLATFORM_WINDOWS
		: _file(INVALID_HANDLE_VALUE)
		, _eof(false)
#endif
	{
	}

	virtual ~DiskFile()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum mode)
	{
#if RIO_PLATFORM_POSIX
		_file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
		RIO_ASSERT(_file != NULL, "fopen: errno = %d, path = '%s'", errno, path);
#elif RIO_PLATFORM_WINDOWS
		_file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, 0
			, NULL
			, OPEN_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		RIO_ASSERT(_file != INVALID_HANDLE_VALUE
			, "CreateFile: GetLastError = %d, path = '%s'"
			, GetLastError()
			, path
			);
#endif
	}

	void close()
	{
		if (is_open())
		{
#if RIO_PLATFORM_POSIX
			fclose(_file);
			_file = NULL;
#elif RIO_PLATFORM_WINDOWS
			CloseHandle(_file);
			_file = INVALID_HANDLE_VALUE;
#endif
		}
	}

	bool is_open() const
	{
#if RIO_PLATFORM_POSIX
		return _file != NULL;
#elif RIO_PLATFORM_WINDOWS
		return _file != INVALID_HANDLE_VALUE;
#endif
	}

	uint32_t getSize()
	{
#if RIO_PLATFORM_POSIX
		long pos = ftell(_file);
		RIO_ASSERT(pos != -1, "ftell: errno = %d", errno);
		int err = fseek(_file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		long size = ftell(_file);
		RIO_ASSERT(size != -1, "ftell: errno = %d", errno);
		err = fseek(_file, (long)pos, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		RIO_UNUSED(err);
		return (uint32_t)size;
#elif RIO_PLATFORM_WINDOWS
		return GetFileSize(_file, NULL);
#endif
	}

	uint32_t getPosition()
	{
#if RIO_PLATFORM_POSIX
		long pos = ftell(_file);
		RIO_ASSERT(pos != -1, "ftell: errno = %d", errno);
		return (uint32_t)pos;
#elif RIO_PLATFORM_WINDOWS
		DWORD pos = SetFilePointer(_file, 0, NULL, FILE_CURRENT);
		RIO_ASSERT(pos != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
		return (uint32_t)pos;
#endif
	}

	bool getIsEndOfFile()
	{
#if RIO_PLATFORM_POSIX
		return feof(_file) != 0;
#elif RIO_PLATFORM_WINDOWS
		return _eof;
#endif
	}

	void seek(uint32_t position)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(_file, (long)position, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, position, NULL, FILE_BEGIN);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	void seekToEnd()
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(_file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, 0, NULL, FILE_END);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	void skip(uint32_t bytes)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(_file, bytes, SEEK_CUR);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, bytes, NULL, FILE_CURRENT);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	uint32_t read(void* data, uint32_t size)
	{
		RIO_ASSERT(data != NULL, "Data must be != NULL");
#if RIO_PLATFORM_POSIX
		size_t bytesRead = fread(data, 1, size, _file);
		RIO_ASSERT(ferror(_file) == 0, "fread error");
		return (uint32_t)bytesRead;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytesRead;
		BOOL err = ReadFile(_file, data, size, &bytesRead, NULL);
		RIO_ASSERT(err == TRUE, "ReadFile: GetLastError = %d", GetLastError());
		_eof = err && bytesRead == 0;
		return bytesRead;
#endif
	}

	uint32_t write(const void* data, uint32_t size)
	{
		RIO_ASSERT(data != NULL, "Data must be != NULL");
#if RIO_PLATFORM_POSIX
		size_t bytes_written = fwrite(data, 1, size, _file);
		RIO_ASSERT(ferror(_file) == 0, "fwrite error");
		return (uint32_t)bytes_written;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(_file, data, size, &bytes_written, NULL);
		RIO_ASSERT(size == bytes_written
			, "WriteFile: GetLastError = %d"
			, GetLastError()
			);
		return bytes_written;
#endif
	}

	void flush()
	{
#if RIO_PLATFORM_POSIX
		int err = fflush(_file);
		RIO_ASSERT(err == 0, "fflush: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = FlushFileBuffers(_file);
		RIO_ASSERT(err != 0
			, "FlushFileBuffers: GetLastError = %d"
			, GetLastError()
			);
#endif
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