// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemDisk.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

class DataCompiler
{
private:
	using CompileFunction = void (*)(const char* path, CompileOptions& compileOptions);

	struct ResourceTypeData
	{
		uint32_t version;
		CompileFunction compileFunction;
	};
public:
	DataCompiler();
	void mapSourceDirectory(const char* name, const char* sourceDirectory);
	void getSourceDirectory(const char* resourceName, DynamicString& sourceDirectory);
	void addIgnoreGlob(const char* glob);
	// Scans the source tree for resources
	void scan();
	// Compiles all the resources found in the source tree and puts them in <dataDirectory>
	// Returns true on success, false otherwise.
	bool compile(const char* dataDirectory, const char* platform);
	// Registers the resource compileFunction for the given resource <type> and <version>
	void registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction);
	// Returns the version of the compiler for <type>
	uint32_t getResourceCompilerVersion(StringId64 type);
private:
	void addFile(const char* path);
	bool canCompile(StringId64 type);
	void compile(StringId64 type, const char* path, CompileOptions& compileOptions);
	void scanSourceDirectory(const char* prefix, const char* path);
	bool compile(FileSystemDisk& bundleFileSystem, const char* type, const char* name, const char* platform);

	FileSystemDisk sourceFileSystem;
	Map<DynamicString, DynamicString> sourceDirectoriesMap;
	SortMap<StringId64, ResourceTypeData> resourceCompilerTable;
	Vector<DynamicString> fileNameList;
	Vector<DynamicString> globList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka