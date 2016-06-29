// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Resource/CompilerTypes.h"
#include "Resource/ResourceTypes.h"

namespace Rio
{

struct ScriptResource
{
	uint32_t version;
	uint32_t size;
//	char program[size]
};

namespace ScriptResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
	// Returns the script program
	const char* getProgram(const ScriptResource* scriptResource);
} // namespace ScriptResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka