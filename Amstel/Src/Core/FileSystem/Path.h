// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Platform.h"
#include "Core/Strings/StringTypes.h"

namespace Rio
{
#if RIO_PLATFORM_POSIX
	const char PATH_SEPARATOR = '/';
#elif RIO_PLATFORM_WINDOWS
	const char PATH_SEPARATOR = '\\';
#endif // RIO_PLATFORM_POSIX

// Functions for operating on strings as file paths
namespace PathFn
{
	bool getIsAbsolute(const char* path);
	bool getIsRelative(const char* path);
	bool getIsRoot(const char* path);
	// Appends <pathB> to <pathA> and fills <path> with the result
	void join(const char* pathA, const char* pathB, DynamicString& path);

	// Returns the basename of the <path>
	// "/home/project/shader.hlsl" => "shader.hlsl"
	// "/home/project" => "project"
	// "/" => ""
	const char* getBasename(const char* path);

	// Returns the extension of the <path> or NULL
	// "/home/shader.hlsl" => "hlsl"
	// "/home/shader" => NULL
	const char* getExtension(const char* path);
} // namespace PathFn
} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka