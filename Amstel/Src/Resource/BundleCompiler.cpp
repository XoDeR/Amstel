// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/BundleCompiler.h"

#include "Config.h"

#include "Core/Memory/Allocator.h"
#include "Core/FileSystem/DiskFileSystem.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/File.h"
#include "Core/Containers/Map.h"
#include "Core/Base/Os.h"
#include "Core/FileSystem/Path.h"
#include "Core/Json/JsonR.h"
#include "Core/Containers/SortMap.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"

#include "Device/ConsoleServer.h"
#include "Device/Log.h"

#include "Resource/CompileOptions.h"

#include <setjmp.h>

namespace Rio
{

class LineReader
{
public:
	LineReader(const char* str)
		: str(str)
		, length(getStringLength32(str))
	{
	}

	void readLine(DynamicString& line)
	{
		const char* s  = &str[position];
		const char* newLine = getNewLineStart(s);
		position += uint32_t(newLine - s);
		line.set(s, newLine - s);
	}

	bool getIsEndOfFile()
	{
		return str[position] == '\0';
	}
private:
	const char* str;
	const uint32_t length;
	uint32_t position = 0;
};

BundleCompiler::BundleCompiler()
	: sourceFileSystem(getDefaultAllocator())
	, resourceCompilerTable(getDefaultAllocator())
	, fileNameList(getDefaultAllocator())
	, globList(getDefaultAllocator())
{
}

void BundleCompiler::scan(const char* sourceDirectory)
{
	sourceFileSystem.setPrefix(sourceDirectory);

	TempAllocator512 ta;
	VectorFn::pushBack(globList, DynamicString("*.tmp", ta));
	VectorFn::pushBack(globList, DynamicString("*.wav", ta));
	VectorFn::pushBack(globList, DynamicString("*.ogg", ta));
	VectorFn::pushBack(globList, DynamicString("*.png", ta));
	VectorFn::pushBack(globList, DynamicString("*.tga", ta));
	VectorFn::pushBack(globList, DynamicString("*.dds", ta));
	VectorFn::pushBack(globList, DynamicString("*.ktx", ta));
	VectorFn::pushBack(globList, DynamicString("*.pvr", ta));
	VectorFn::pushBack(globList, DynamicString("*.swn", ta)); // VIM swap file.
	VectorFn::pushBack(globList, DynamicString("*.swo", ta)); // VIM swap file.
	VectorFn::pushBack(globList, DynamicString("*.swp", ta)); // VIM swap file.
	VectorFn::pushBack(globList, DynamicString("*~", ta));
	VectorFn::pushBack(globList, DynamicString(".*", ta));

	if (sourceFileSystem.getDoesExist(RIO_BUNDLEIGNORE))
	{
		File& file = *sourceFileSystem.open(RIO_BUNDLEIGNORE, FileOpenMode::READ);
		const uint32_t size = file.getSize();
		char* data = (char*)getDefaultAllocator().allocate(size + 1);
		file.read(data, size);
		data[size] = '\0';
		sourceFileSystem.close(file);

		LineReader lineReader(data);

		while (!lineReader.getIsEndOfFile())
		{
			TempAllocator512 ta;
			DynamicString line(ta);
			lineReader.readLine(line);

			line.trim();

			if (line.getIsEmpty() || line.hasPrefix("#"))
			{
				continue;
			}

			VectorFn::pushBack(globList, line);
		}

		getDefaultAllocator().deallocate(data);
	}

	scanSourceDirectory("");
}

bool BundleCompiler::compile(DiskFileSystem& bundleFileSystem, const char* type, const char* name, const char* platform)
{
	StringId64 resourceType(type);
	StringId64 resourceName(name);

	TempAllocator1024 ta;
	DynamicString path(ta);
	DynamicString sourcePath(ta);

	// Build source file path
	sourcePath += name;
	sourcePath += '.';
	sourcePath += type;

	// Build compiled file path
	DynamicString resourceTypeStr(ta);
	DynamicString resourceNameStr(ta);
	resourceType.toString(resourceTypeStr);
	resourceName.toString(resourceNameStr);

	DynamicString destinationPath(ta);
	destinationPath += resourceTypeStr;
	destinationPath += '-';
	destinationPath += resourceNameStr;

	PathFn::join(RIO_DATA_DIRECTORY, destinationPath.getCStr(), path);

	RIO_LOGI("%s <= %s", destinationPath.getCStr(), sourcePath.getCStr());

	bool success = true;
	jmp_buf jmpBuffer;

	Buffer output(getDefaultAllocator());
	ArrayFn::reserve(output, 4*1024*1024);

	if (!setjmp(jmpBuffer))
	{
		CompileOptions compileOptions(sourceFileSystem, bundleFileSystem, output, platform, &jmpBuffer);
		compile(resourceType, sourcePath.getCStr(), compileOptions);
		File* outputFile = bundleFileSystem.open(path.getCStr(), FileOpenMode::WRITE);
		uint32_t size = ArrayFn::getCount(output);
		uint32_t written = outputFile->write(ArrayFn::begin(output), size);
		bundleFileSystem.close(*outputFile);
		success = size == written;
	}
	else
	{
		success = false;
	}

	return success;
}

bool BundleCompiler::compile(const char* bundleDirectory, const char* platform)
{
	// Create bundle directory if necessary
	DiskFileSystem bundleFileSystem(getDefaultAllocator());
	bundleFileSystem.setPrefix(bundleDirectory);
	bundleFileSystem.createDirectory("");

	if (!bundleFileSystem.getDoesExist(RIO_DATA_DIRECTORY))
	{
		bundleFileSystem.createDirectory(RIO_DATA_DIRECTORY);
	}

	if (!bundleFileSystem.getDoesExist(RIO_TEMP_DIRECTORY))
	{
		bundleFileSystem.createDirectory(RIO_TEMP_DIRECTORY);
	}

	// Compile all changed resources
	for (uint32_t i = 0; i < VectorFn::getCount(fileNameList); ++i)
	{
		const char* fileName = fileNameList[i].getCStr();

		bool ignore = false;
		for (uint32_t globIndex = 0; globIndex < VectorFn::getCount(globList); ++globIndex)
		{
			const char* glob = globList[globIndex].getCStr();

			if (getWildCardPatternMatch(glob, fileName))
			{
				ignore = true;
				break;
			}
		}

		const char* type = PathFn::getExtension(fileName);

		if (ignore || type == nullptr)
		{
			continue;
		}

		char name[256];
		const uint32_t size = uint32_t(type - fileName - 1);
		strncpy(name, fileName, size);
		name[size] = '\0';

		if (!compile(bundleFileSystem, type, name, platform))
		{
			return false;
		}
	}

	return true;
}

void BundleCompiler::registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction)
{
	RIO_ASSERT(!SortMapFn::has(resourceCompilerTable, type), "Type already registered");
	RIO_ASSERT_NOT_NULL(compileFunction);

	ResourceTypeData resourceTypeData;
	resourceTypeData.version = version;
	resourceTypeData.compileFunction = compileFunction;

	SortMapFn::set(resourceCompilerTable, type, resourceTypeData);
	SortMapFn::sort(resourceCompilerTable);
}

void BundleCompiler::compile(StringId64 type, const char* path, CompileOptions& compileOptions)
{
	RIO_ASSERT(SortMapFn::has(resourceCompilerTable, type), "Compiler not found");

	SortMapFn::get(resourceCompilerTable, type, ResourceTypeData()).compileFunction(path, compileOptions);
}

uint32_t BundleCompiler::getResourceCompilerVersion(StringId64 type)
{
	RIO_ASSERT(SortMapFn::has(resourceCompilerTable, type), "Compiler not found");

	return SortMapFn::get(resourceCompilerTable, type, ResourceTypeData()).version;
}

void BundleCompiler::scanSourceDirectory(const char* currentDirectory)
{
	Vector<DynamicString> fileNameList(getDefaultAllocator());
	sourceFileSystem.getFileList(currentDirectory, fileNameList);

	for (uint32_t i = 0; i < VectorFn::getCount(fileNameList); ++i)
	{
		TempAllocator512 ta;
		DynamicString fileName(ta);

		if (strcmp(currentDirectory, "") != 0)
		{
			fileName += currentDirectory;
			fileName += '/';
		}
		fileName += fileNameList[i];

		if (sourceFileSystem.getIsDirectory(fileName.getCStr()))
		{
			BundleCompiler::scanSourceDirectory(fileName.getCStr());
		}
		else // Assume a regular file
		{
			VectorFn::pushBack(this->fileNameList, fileName);
		}
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka