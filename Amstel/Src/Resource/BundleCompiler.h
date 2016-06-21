// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Resource/CompileOptions.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/DiskFileSystem.h"

namespace Rio
{

class BundleCompiler
{
private:
	using CompileFunction = void (*)(const char* path, CompileOptions& opts);

	struct ResourceTypeData
	{
		uint32_t version;
		CompileFunction compileFunction;
	};
public:
	BundleCompiler();
	// Scans <sourceDirectory> for resources
	void scan(const char* sourceDirectory);
	// Compiles all the resources found in <sourceDirectory> and puts them in <bundleDirectory>
	// Returns true on success, false otherwise.
	bool compile(const char* bundleDirectory, const char* platform);
	// Registers the resource compileFunction for the given resource <type> and <version>
	void registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction);
	// Returns the version of the compiler for <type>
	uint32_t getResourceCompilerVersion(StringId64 type);
private:
	void compile(StringId64 type, const char* path, CompileOptions& opts);
	void scanSourceDirectory(const char* path);
	bool compile(DiskFileSystem& bundleFileSystem, const char* type, const char* name, const char* platform);

	DiskFileSystem sourceFileSystem;
	SortMap<StringId64, ResourceTypeData> resourceCompilerTable;
	Vector<DynamicString> fileNameList;
	Vector<DynamicString> globList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka