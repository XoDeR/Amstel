// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/Vector.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/Path.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Base/Guid.h"
#include "Core/Memory/TempAllocator.h"

#include "Device/Log.h"

#include "Resource/DataCompiler.h"

#include <setjmp.h> // jmp_buf

#define RESOURCE_COMPILER_ASSERT(condition, compileOptions, msg, ...) do { if (!(condition))\
	{ compileOptions.error(msg, ##__VA_ARGS__); } } while(0)

#define RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(type, name, compileOptions)\
	RESOURCE_COMPILER_ASSERT(compileOptions.doesResourceExist(type, name), compileOptions, "Resource does not exist: '%s.%s'", name, type)

#define RESOURCE_COMPILER_ASSERT_FILE_EXISTS(name, compileOptions)\
	RESOURCE_COMPILER_ASSERT(compileOptions.doesFileExist(name), compileOptions, "File does not exist: '%s'", name)

namespace Rio
{

class CompileOptions
{
public:
	CompileOptions(DataCompiler& dataCompiler, FileSystem& bundleFileSystem, Buffer& outputBuffer, const char* platformName, jmp_buf* jmpBuf)
		: dataCompiler(dataCompiler)
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
		TempAllocator256 ta;
		DynamicString sourceDirectory(ta);
		FileSystemDisk fileSystemDisk(ta);

		dataCompiler.getSourceDirectory(path, sourceDirectory);
		fileSystemDisk.setPrefix(sourceDirectory.getCStr());

		return fileSystemDisk.getDoesExist(path);
	}

	bool doesResourceExist(const char* type, const char* name)
	{
		TempAllocator1024 ta;
		DynamicString path(ta);
		path += name;
		path += ".";
		path += type;
		return doesFileExist(path.getCStr());
	}

	Buffer readTemporary(const char* path)
	{
		File* file = bundleFileSystem.open(path, FileOpenMode::READ);
		uint32_t size = file->getSize();
		Buffer buffer(getDefaultAllocator());
		ArrayFn::resize(buffer, size);
		file->read(ArrayFn::begin(buffer), size);
		bundleFileSystem.close(*file);
		return buffer;
	}

	void writeTemporary(const char* path, const char* data, uint32_t size)
	{
		File* file = bundleFileSystem.open(path, FileOpenMode::WRITE);
		file->write(data, size);
		bundleFileSystem.close(*file);
	}

	void writeTemporary(const char* path, const Buffer& data)
	{
		writeTemporary(path, ArrayFn::begin(data), ArrayFn::getCount(data));
	}

	Buffer read(const char* path)
	{
		addDependency(path);

		TempAllocator256 ta;
		DynamicString sourceDirectoryStr(ta);
		FileSystemDisk sourceFileSystem(ta);

		dataCompiler.getSourceDirectory(path, sourceDirectoryStr);
		sourceFileSystem.setPrefix(sourceDirectoryStr.getCStr());

		File* file = sourceFileSystem.open(path, FileOpenMode::READ);
		uint32_t size = file->getSize();
		Buffer buffer(getDefaultAllocator());
		ArrayFn::resize(buffer, size);
		file->read(ArrayFn::begin(buffer), size);
		sourceFileSystem.close(*file);
		return buffer;
	}

	void getAbsolutePath(const char* path, DynamicString& absolutePath)
	{
		TempAllocator256 ta;
		DynamicString sourceDirectoryStr(ta);
		FileSystemDisk sourceFileSystem(ta);
		dataCompiler.getSourceDirectory(path, sourceDirectoryStr);
		sourceFileSystem.setPrefix(sourceDirectoryStr.getCStr());
		sourceFileSystem.getAbsolutePath(path, absolutePath);
	}

	void getTemporaryPath(const char* suffix, DynamicString& absolutePath)
	{
		bundleFileSystem.getAbsolutePath(RIO_TEMP_DIRECTORY, absolutePath);

		TempAllocator64 ta;
		DynamicString prefix(ta);
		GuidFn::toString(GuidFn::createGuid(), prefix);

		absolutePath += '/';
		absolutePath += prefix;
		absolutePath += '.';
		absolutePath += suffix;
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
		DynamicString dependency(ta);
		dependency += path;
		VectorFn::pushBack(dependencyList, dependency);
	}

private:
	DataCompiler& dataCompiler;
	FileSystem& bundleFileSystem;
	Buffer& outputBuffer;
	const char* platformName;
	jmp_buf* jmpBuf;
	Vector<DynamicString> dependencyList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka