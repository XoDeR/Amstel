// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/DataCompiler.h"

#include "Config.h"

#include "Core/Memory/Allocator.h"
#include "Core/Memory/TempAllocator.h"

#include "Core/FileSystem/File.h"
#include "Core/FileSystem/Path.h"
#include "Core/FileSystem/FileSystemDisk.h"

#include "Core/Strings/DynamicString.h"

#include "Core/Containers/Map.h"
#include "Core/Containers/SortMap.h"
#include "Core/Containers/Vector.h"

#include "Core/Base/Os.h"

#include "Core/Json/JsonR.h"

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

DataCompiler::DataCompiler()
	: sourceFileSystem(getDefaultAllocator())
	, sourceDirectories(getDefaultAllocator())
	, resourceCompilerTable(getDefaultAllocator())
	, fileNameList(getDefaultAllocator())
	, globList(getDefaultAllocator())
{
}

void DataCompiler::addFile(const char* path)
{
	for (u32 gg = 0; gg < vector::size(_globs); ++gg)
	{
		if (wildcmp(_globs[gg].c_str(), path))
		{
			return;
		}
	}

	TempAllocator512 ta;
	DynamicString str(ta);
	str.set(path, strlen32(path));
	VectorFn::pushBack(fileNameList, str);
}

bool DataCompiler::canCompile(StringId64 type)
{
	return SortMapFn::has(resourceCompilerTable, type);
}

void DataCompiler::compile(StringId64 type, const char* path, CompileOptions& compileOptions)
{
	RIO_ASSERT(SortMapFn::has(resourceCompilerTable, type), "Compiler not found");

	SortMapFn::get(resourceCompilerTable, type, ResourceTypeData()).compileFunction(path, compileOptions);
}

void DataCompiler::scanSourceDirectory(const char* prefix, const char* currentDirectory)
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
			DataCompiler::scanSourceDirectory(prefix, fileName.getCStr());
		}
		else // Assume a regular file
		{
			DynamicString resourceName(ta);
			if (strcmp(prefix, "") != 0)
			{
				resourceName += prefix;
				resourceName += '/';
			}
			resourceName += fileName;
			addFile(resourceName.getCStr());
		}
	}
}

bool DataCompiler::compile(FileSystemDisk& bundleFileSystem, const char* type, const char* name, const char* platform)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	DynamicString sourcePath(ta);

	DynamicString resourceTypeStr(ta);
	DynamicString resourceNameStr(ta);
	DynamicString destinationPath(ta);

	StringId64 resourceType(type);
	StringId64 resourceName(name);

	// Build source file path
	sourcePath += name;
	sourcePath += '.';
	sourcePath += type;

	// Build compiled file path
	resourceType.toString(resourceTypeStr);
	resourceName.toString(resourceNameStr);

	// Build destination file path
	destinationPath += resourceTypeStr;
	destinationPath += '-';
	destinationPath += resourceNameStr;

	PathFn::join(RIO_DATA_DIRECTORY, destinationPath.getCStr(), path);

	RIO_LOGI("%s <= %s", destinationPath.getCStr(), sourcePath.getCStr());
	if (!canCompile(resourceType))
	{
		RIO_LOGE("Unknown resource type: '%s'", type);
		return false;
	}

	bool success = true;
	jmp_buf jmpBuffer;

	Buffer output(getDefaultAllocator());
	ArrayFn::reserve(output, 4 * 1024 * 1024);

	if (!setjmp(jmpBuffer))
	{
		CompileOptions compileOptions(*this, bundleFileSystem, output, platform, &jmpBuffer);
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

void DataCompiler::mapSourceDirectory(const char* name, const char* source_dir)
{
	TempAllocator256 ta;
	DynamicString sname(ta);
	DynamicString sdir(ta);
	sname.set(name, strlen32(name));
	sdir.set(source_dir, strlen32(source_dir));
	map::set(_source_dirs, sname, sdir);
}

void DataCompiler::getSourceDirectory(const char* resource_name, DynamicString& source_dir)
{
	const char* slash = strchr(resource_name, '/');

	TempAllocator256 ta;
	DynamicString source_name(ta);

	if (slash != NULL)
	{
		source_name.set(resource_name, (slash - resource_name));
	}
	else
	{
		source_name.set("", 0);
	}

	DynamicString deffault(ta);
	DynamicString empty(ta);
	empty = "";

	deffault = map::get(_source_dirs, empty, empty);
	source_dir = map::get(_source_dirs, source_name, deffault);
}

void DataCompiler::addIgnoreGlob(const char* glob)
{
	TempAllocator64 ta;
	DynamicString str(ta);
	str.set(glob, strlen32(glob));
	VectorFn::pushBack(_globs, str);
}

void DataCompiler::scan()
{
	add_ignore_glob("*.tmp");
	add_ignore_glob("*.wav");
	add_ignore_glob("*.ogg");
	add_ignore_glob("*.png");
	add_ignore_glob("*.tga");
	add_ignore_glob("*.dds");
	add_ignore_glob("*.ktx");
	add_ignore_glob("*.pvr");
	add_ignore_glob("*.swn"); // VIM swap file.
	add_ignore_glob("*.swo"); // VIM swap file.
	add_ignore_glob("*.swp"); // VIM swap file.
	add_ignore_glob("*.bak");
	add_ignore_glob("*~");
	add_ignore_glob(".*");

	// Scan all source directories
	auto current = MapFn::begin(_source_dirs);
	auto end = MapFn::end(_source_dirs);

	for (; current != end; ++current)
	{
		DynamicString prefix(default_allocator());
		prefix += cur->pair.second.c_str();
		prefix += '/';
		prefix += cur->pair.first.c_str();
		sourceFileSystem.set_prefix(prefix.c_str());

		if (sourceFileSystem.exists(CROWN_BUNDLEIGNORE))
		{
			File& file = *_source_fs.open(CROWN_BUNDLEIGNORE, FileOpenMode::READ);
			const u32 size = file.size();
			char* data = (char*)default_allocator().allocate(size + 1);
			file.read(data, size);
			data[size] = '\0';
			_source_fs.close(file);

			LineReader lr(data);

			while (!lr.eof())
			{
				TempAllocator512 ta;
				DynamicString line(ta);
				lr.read_line(line);

				line.trim();

				if (line.empty() || line.has_prefix("#"))
				{
					continue;
				}
				add_ignore_glob(line.c_str());
			}

			getDefaultAllocator().deallocate(data);
		}

		scanSourceDirectory(current->pair.first.c_str(), "");
	}
}

bool DataCompiler::compile(const char* dataDirectory, const char* platform)
{
	// Create bundle directory if necessary
	FileSystemDisk bundleFileSystem(getDefaultAllocator());
	bundleFileSystem.setPrefix(dataDirectory);
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
		const char* type = PathFn::getExtension(fileName);
		if (type == nullptr)
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

void DataCompiler::registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction)
{
	RIO_ASSERT(!SortMapFn::has(resourceCompilerTable, type), "Type already registered");
	RIO_ASSERT_NOT_NULL(compileFunction);

	ResourceTypeData resourceTypeData;
	resourceTypeData.version = version;
	resourceTypeData.compileFunction = compileFunction;

	SortMapFn::set(resourceCompilerTable, type, resourceTypeData);
	SortMapFn::sort(resourceCompilerTable);
}



uint32_t DataCompiler::getResourceCompilerVersion(StringId64 type)
{
	RIO_ASSERT(SortMapFn::has(resourceCompilerTable, type), "Compiler not found");

	return SortMapFn::get(resourceCompilerTable, type, ResourceTypeData()).version;
}



} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka