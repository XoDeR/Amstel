// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/Path.h"

#include <ctype.h> // isalpha
#include <string.h> // strrchr

namespace Rio
{

namespace PathFn
{
	bool getIsAbsolute(const char* path)
	{
		RIO_ASSERT_NOT_NULL(path);
#if RIO_PLATFORM_POSIX
		return getStringLength32(path) > 0
			&& path[0] == PATH_SEPARATOR
			;
#elif RIO_PLATFORM_WINDOWS
		return getStringLength32(path) > 2
			&& isalpha(path[0])
			&& path[1] == ':'
			&& path[2] == PATH_SEPARATOR
			;
#endif
	}

	bool getIsRelative(const char* path)
	{
		RIO_ASSERT_NOT_NULL(path);
		return !getIsAbsolute(path);
	}

	bool getIsRoot(const char* path)
	{
		RIO_ASSERT_NOT_NULL(path);
#if RIO_PLATFORM_POSIX
		return getIsAbsolute(path) && getStringLength32(path) == 1;
#elif RIO_PLATFORM_WINDOWS
		return getIsAbsolute(path) && getStringLength32(path) == 3;
#endif // RIO_PLATFORM_
	}

	void join(const char* pathA, const char* pathB, DynamicString& path)
	{
		RIO_ASSERT_NOT_NULL(pathA);
		RIO_ASSERT_NOT_NULL(pathB);
		const uint32_t lengthA = getStringLength32(pathA);
		const uint32_t lengthB = getStringLength32(pathB);
		path.reserve(lengthA + lengthB + 1);
		path += pathA;
		path += PATH_SEPARATOR;
		path += pathB;
	}

	const char* getBasename(const char* path)
	{
		RIO_ASSERT_NOT_NULL(path);
		const char* shift = strrchr(path, '/');
		return shift == nullptr ? path : shift + 1;
	}

	const char* getExtension(const char* path)
	{
		RIO_ASSERT_NOT_NULL(path);
		const char* dot = strrchr(path, '.');
		return dot == nullptr ? nullptr : dot + 1;
	}
} // namespace PathFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka