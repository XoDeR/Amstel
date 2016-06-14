// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Base/Platform.h"

#include <stdio.h> // sscanf, vsnprintf
#include <string.h> // strlen
#include <stdarg.h>
#include <ctype.h> // isspace

namespace Rio
{

inline int32_t vsnPrintF(char* str, size_t num, const char* format, va_list args)
{
#if RIO_COMPILER_MSVC
	int32_t length = _vsnprintf_s(str, num, _TRUNCATE, format, args);
	return (length == 1) ? _vscprintf(format, args) : length;
#else
	return ::vsnprintf(str, num, format, args);
#endif // RIO_COMPILER_MSVC
}

inline int32_t snPrintF(char* str, size_t n, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int32_t length = vsnPrintF(str, n, format, args);
	va_end(args);
	return length;
}

inline uint32_t getStringLength32(const char* str)
{
	return (uint32_t)strlen(str);
}

inline const char* skipSpaces(const char* str)
{
	while (isspace(*str)) ++str;
	return str;
}

inline const char* skipBlock(const char* str, char a, char b)
{
	uint32_t num = 0;

	for (char ch = *str++; ch != '\0'; ch = *str++)
	{
		if (ch == a) ++num;
		else if (ch == b)
		{
			if (--num == 0)
			{
				return str;
			}
		}
	}

	return nullptr;
}

// Returns pointer after EOL
inline const char* getNewLineStart(const char* str)
{
	const char* eol = strchr(str, '\n');
	return eol ? eol + 1 : str + strlen(str);
}

inline int getWildCardPatternMatch(const char* wild, const char* str)
{
	const char* cp = nullptr;
	const char* mp = nullptr;

	while (*str && *wild != '*')
	{
		if (*wild != *str && *wild != '?')
		{
			return 0;
		}
		++wild;
		++str;
	}

	while (*str)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = str + 1;
		}
		else if (*wild == *str || *wild == '?')
		{
			++wild;
			++str;
		}
		else
		{
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*')
	{
		++wild;
	}

	return !*wild;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka