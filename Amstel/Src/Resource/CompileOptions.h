// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/Array.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Base/Guid.h"
#include "Core/FileSystem/Path.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"
#include "Device/Log.h"

#include <setjmp.h> // jmp_buf

#define RESOURCE_COMPILER_ASSERT(condition, opts, msg, ...) do { if (!(condition))\
	{ opts.error(msg, ##__VA_ARGS__); } } while(0)

#define RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(type, name, opts)\
	RESOURCE_COMPILER_ASSERT(opts.doesResourceExist(type, name), opts, "Resource does not exist: '%s.%s'", name, type)

#define RESOURCE_COMPILER_ASSERT_FILE_EXISTS(name, opts)\
	RESOURCE_COMPILER_ASSERT(opts.doesFileExist(name), opts, "File does not exist: '%s'", name)

namespace Rio
{

class CompileOptions
{
public:
	CompileOptions(FileSystem& source_fs, FileSystem& bundleFileSystem, Buffer& outputBuffer, const char* platformName, jmp_buf* jmpBuf)
		: sourceFileSystem(source_fs)
		, bundleFileSystem(bundleFileSystem)
		, outputBuffer(outputBuffer)
		, platformName(platformName)
		, jmpBuf(jmpBuf)
		, dependencyList(getDefaultAllocator())
	{
	}

	void error(const char* msg, va_list args)
	{
		RIO_LOGEV(msg, args);
		longjmp(*jmpBuf, 1);
	}

	void error(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		error(msg, args);
		va_end(args);
	}

	bool doesFileExist(const char* path)
	{
		return sourceFileSystem.getDoesExist(path);
	}

	bool doesResourceExist(const char* type, const char* name)
	{
		TempAllocator1024 ta;
		DynamicString path(name, ta);
		path += ".";
		path += type;
		return doesFileExist(path.getCStr());
	}

	Buffer readTemporary(const char* path)
	{
		File* file = bundleFileSystem.open(path, FileOpenMode::READ);
		uint32_t size = file->getSize();
		Buffer buf(getDefaultAllocator());
		ArrayFn::resize(buf, size);
		file->read(ArrayFn::begin(buf), size);
		bundleFileSystem.close(*file);
		return buf;
	}

	void writeTemporary(const char* path, const char* data, uint32_t size)
	{
		File* file = sourceFileSystem.open(path, FileOpenMode::WRITE);
		file->write(data, size);
		sourceFileSystem.close(*file);
	}

	void writeTemporary(const char* path, const Buffer& data)
	{
		writeTemporary(path, ArrayFn::begin(data), ArrayFn::getCount(data));
	}

	Buffer read(const char* path)
	{
		addDependency(path);

		File* file = sourceFileSystem.open(path, FileOpenMode::READ);
		uint32_t size = file->getSize();
		Buffer buf(getDefaultAllocator());
		ArrayFn::resize(buf, size);
		file->read(ArrayFn::begin(buf), size);
		sourceFileSystem.close(*file);
		return buf;
	}

	void getAbsolutePath(const char* path, DynamicString& abs)
	{
		sourceFileSystem.getAbsolutePath(path, abs);
	}

	void getTemporaryPath(const char* suffix, DynamicString& abs)
	{
		bundleFileSystem.getAbsolutePath(RIO_TEMP_DIRECTORY, abs);

		TempAllocator64 ta;
		DynamicString prefix(ta);
		GuidFn::toString(GuidFn::createGuid(), prefix);

		abs += '/';
		abs += prefix;
		abs += '.';
		abs += suffix;
	}

	void deleteFile(const char* path)
	{
		bundleFileSystem.deleteFile(path);
	}

	void write(const void* data, uint32_t size)
	{
		ArrayFn::push(outputBuffer, (const char*)data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		write(&data, sizeof(data));
	}

	void write(const Buffer& data)
	{
		ArrayFn::push(outputBuffer, ArrayFn::begin(data), ArrayFn::getCount(data));
	}

	const char* getPlatform() const
	{
		return platformName;
	}

	const Vector<DynamicString>& getDependencies() const
	{
		return dependencyList;
	}

	void addDependency(const char* path)
	{
		TempAllocator256 ta;
		DynamicString dependency(path, ta);
		VectorFn::pushBack(dependencyList, dependency);
	}

private:
	FileSystem& sourceFileSystem;
	FileSystem& bundleFileSystem;
	Buffer& outputBuffer;
	const char* platformName;
	jmp_buf* jmpBuf;
	Vector<DynamicString> dependencyList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka