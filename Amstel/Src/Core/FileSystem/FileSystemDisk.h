// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/FileSystem.h"

namespace Rio
{

// Access files on disk
// All the file paths can be either relative or absolute
// Relative paths are translated to absolute based on the file source's root path
// Absolute paths are platform-specific
class FileSystemDisk : public FileSystem
{
public:
	FileSystemDisk(Allocator& a);
	// Sets the root path to the given prefix
	// The <prefix> must be absolute
	void setPrefix(const char* prefix);
	File* open(const char* path, FileOpenMode::Enum mode);
	void close(File& file);
	bool getDoesExist(const char* path);
	bool getIsDirectory(const char* path);
	bool getIsFile(const char* path);
	uint64_t getLastModifiedTime(const char* path);
	void createDirectory(const char* path);
	void deleteDirectory(const char* path);
	void createFile(const char* path);
	void deleteFile(const char* path);
	void getFileList(const char* path, Vector<DynamicString>& files);
	void getAbsolutePath(const char* path, DynamicString& osPath);
private:
	Allocator* allocator;
	DynamicString prefix;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka