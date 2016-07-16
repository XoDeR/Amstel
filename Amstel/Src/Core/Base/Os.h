// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Base/Macros.h"
#include "Core/Base/Platform.h"
#include "Core/Strings/StringTypes.h"
#include "Core/Base/Types.h"

#include <stdio.h>  // fputs

#if RIO_PLATFORM_POSIX
	#include <dlfcn.h> // dlopen, dlclose, dlsym
	#include <errno.h>
	#include <string.h> // memset
	#include <sys/stat.h> // lstat, mknod, mkdir
	#include <sys/wait.h> // wait
	#include <time.h> // clock_gettime
	#include <unistd.h> // access, unlink, rmdir, getcwd, fork, execv
 	#include <stdlib.h> // exit
#elif RIO_PLATFORM_WINDOWS
	#include <io.h>
	#include "Device/Windows/Headers_Windows.h"
#endif // RIO_PLATFORM_
#if RIO_PLATFORM_ANDROID
	#include <android/log.h>
#endif // RIO_PLATFORM_ANDROID

namespace Rio
{

namespace OsFn
{
	inline int64_t getClockTime()
	{
#if RIO_PLATFORM_LINUX || RIO_PLATFORM_ANDROID
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * int64_t(1000000000) + now.tv_nsec;
#elif RIO_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * int64_t(1000000) + now.tv_usec;
#elif RIO_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (int64_t)ttime.QuadPart;
#endif // RIO_PLATFORM_
	}

	inline int64_t getClockFrequency()
	{
#if RIO_PLATFORM_LINUX || RIO_PLATFORM_ANDROID
		return int64_t(1000000000);
#elif RIO_PLATFORM_OSX
		return int64_t(1000000);
#elif RIO_PLATFORM_WINDOWS
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return (int64_t)frequency.QuadPart;
#endif // RIO_PLATFORM_
	}

	inline void sleep(uint32_t milliSeconds)
	{
#if RIO_PLATFORM_POSIX
		usleep(milliSeconds * 1000);
#elif RIO_PLATFORM_WINDOWS
		Sleep(milliSeconds);
#endif // RIO_PLATFORM_
	}

	inline void* openLibrary(const char* path)
	{
#if RIO_PLATFORM_POSIX
		return ::dlopen(path, RTLD_LAZY);
#elif RIO_PLATFORM_WINDOWS
		return (void*)LoadLibraryA(path);
#endif // RIO_PLATFORM_
	}

	inline void closeLibrary(void* library)
	{
#if RIO_PLATFORM_POSIX
		dlclose(library);
#elif RIO_PLATFORM_WINDOWS
		FreeLibrary((HMODULE)library);
#endif // RIO_PLATFORM_
	}

	inline void* lookupSymbol(void* library, const char* name)
	{
#if RIO_PLATFORM_POSIX
		return ::dlsym(library, name);
#elif RIO_PLATFORM_WINDOWS
		return (void*)GetProcAddress((HMODULE)library, name);
#endif // RIO_PLATFORM_
	}

	inline void log(const char* msg)
	{
#if RIO_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "Rio", msg);
#else
		fputs(msg, stdout);
		fflush(stdout);
#endif // RIO_PLATFORM_
	}

	// Returns whether the <path> exists
	inline bool getDoesExist(const char* path)
	{
#if RIO_PLATFORM_POSIX
		return access(path, F_OK) != -1;
#elif RIO_PLATFORM_WINDOWS
		return _access(path, 0) != -1; // <corecrt_io.h>
#endif // RIO_PLATFORM_
	}

	// Returns whether <path> is a directory
	inline bool getIsDirectory(const char* path)
	{
#if RIO_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
		RIO_UNUSED(err);
		return ((S_ISDIR(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif RIO_PLATFORM_WINDOWS
		DWORD fileAttributes = GetFileAttributes(path);
		return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#endif // RIO_PLATFORM_
	}

	// Is a regular file
	inline bool getIsFile(const char* path)
	{
#if RIO_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
		RIO_UNUSED(err);
		return ((S_ISREG(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif RIO_PLATFORM_WINDOWS
		DWORD fileAttributes = GetFileAttributes(path);
		return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
#endif // RIO_PLATFORM_
	}

	// Returns the last modification time of the path
	inline uint64_t getLastModifiedTime(const char* path)
	{
#if RIO_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
		RIO_UNUSED(err);
		return info.st_mtime;
#elif RIO_PLATFORM_WINDOWS
		HANDLE fileHandle = CreateFile(path
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, 0
			, NULL
			);
		RIO_ASSERT(fileHandle != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
		FILETIME fileTimeWrite;
		BOOL err = GetFileTime(fileHandle, NULL, NULL, &fileTimeWrite);
		RIO_ASSERT(err != 0, "GetFileTime: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
		CloseHandle(fileHandle);
		return (uint64_t)((uint64_t(fileTimeWrite.dwHighDateTime) << 32) | fileTimeWrite.dwLowDateTime);
#endif // RIO_PLATFORM_
	}

	inline void createFile(const char* path)
	{
#if RIO_PLATFORM_POSIX
		// Permission mask: rw-r--r--
		int err = ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0);
		RIO_ASSERT(err == 0, "mknod: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		HANDLE fileHandle = CreateFile(path
			, GENERIC_READ | GENERIC_WRITE
			, 0
			, NULL
			, CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		RIO_ASSERT(fileHandle != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
		CloseHandle(fileHandle);
#endif // RIO_PLATFORM_
	}

	inline void deleteFile(const char* path)
	{
#if RIO_PLATFORM_POSIX
		int err = ::unlink(path);
		RIO_ASSERT(err == 0, "unlink: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = DeleteFile(path);
		RIO_ASSERT(err != 0, "DeleteFile: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

	inline void createDirectory(const char* path)
	{
#if RIO_PLATFORM_POSIX
		// rwxr-xr-x
		int err = ::mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		RIO_ASSERT(err == 0, "mkdir: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = CreateDirectory(path, NULL);
		RIO_ASSERT(err != 0, "CreateDirectory: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

	inline void deleteDirectory(const char* path)
	{
#if RIO_PLATFORM_POSIX
		int err = ::rmdir(path);
		RIO_ASSERT(err == 0, "rmdir: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = RemoveDirectory(path);
		RIO_ASSERT(err != 0, "RemoveDirectory: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif // RIO_PLATFORM_
	}

	void getFileList(const char* path, Vector<DynamicString>& files);

	// Returns the current working directory
	inline const char* getCurrentWorkingDirectory(char* buffer, uint32_t size)
	{
#if RIO_PLATFORM_POSIX
		return ::getcwd(buffer, size);
#elif RIO_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buffer);
		return buffer;
#endif // RIO_PLATFORM_
	}

	// Returns the value of the environment variable <name>
	inline const char* getEnvironmentVariable(const char* name)
	{
#if RIO_PLATFORM_POSIX
		return ::getenv(name);
#elif RIO_PLATFORM_WINDOWS
		// TODO
		// GetEnvironmentVariable(name, buffer, size);
#endif // RIO_PLATFORM_
	}

	// Executes the process <path> with the given <args> and returns its exit code
	// Fills <output> with stdout and stderr
	int executeProcess(const char* path, const char* args, StringStream& output);
} // namespace OsFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka