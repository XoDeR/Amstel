// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringTypes.h"

namespace Rio
{

class FileSystem
{
public:
	FileSystem() {};
	virtual ~FileSystem() {};

	virtual File* open(const char* path, FileOpenMode::Enum mode) = 0;
	virtual void close(File& file) = 0;
	virtual bool getDoesExist(const char* path) = 0;
	virtual bool getIsDirectory(const char* path) = 0;
	virtual bool getIsFile(const char* path) = 0;
	virtual uint64_t getLastModifiedTime(const char* path) = 0;
	virtual void createDirectory(const char* path) = 0;
	virtual void deleteDirectory(const char* path) = 0;
	virtual void createFile(const char* path) = 0;
	virtual void deleteFile(const char* path) = 0;
	virtual void getFileList(const char* path, Vector<DynamicString>& files) = 0;
	// Returns the absolute path of the given path based on the root path of the file source
	// If <path> is absolute, the given path is returned
	virtual void getAbsolutePath(const char* path, DynamicString& osPath) = 0;
private:
	// Disable copying
	FileSystem(const FileSystem&);
	FileSystem& operator=(const FileSystem&);
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka