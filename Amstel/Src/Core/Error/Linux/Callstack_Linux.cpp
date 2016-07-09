// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_LINUX && RIO_COMPILER_GCC

#include "Core/Base/Macros.h"
#include "Core/Strings/StringUtils.h"
#include "Device/Log.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h> // strchr
#include <unistd.h> // getpid

namespace Rio
{

namespace ErrorFn
{
	static const char* addr2line(const char* addr, char* line, int length)
	{
		char buffer[256];
		snPrintF(buffer, sizeof(buffer), "addr2line -e /proc/%u/exe %s", getpid(), addr);
		FILE* file = popen(buffer, "r");
		if (file != nullptr)
		{
			fgets(line, length, file);
			line[getStringLength32(line) - 1] = '\0';
			pclose(file);
			return line;
		}
		return "<addr2line missing>";
	}

	void printCallstack()
	{
		void* array[64];
		int size = backtrace(array, RIO_COUNTOF(array));
		char** messages = backtrace_symbols(array, size);

		// skip first stack frame (points here)
		for (int i = 1; i < size && messages != NULL; ++i)
		{
			char* message = messages[i];
			char* mangledName = strchr(message, '(');
			char* offsetBegin = strchr(message, '+');
			char* offsetEnd = strchr(message, ')');
			char* addressBegin = strchr(message, '[');
			char* addressEnd = strchr(message, ']');

			// if the line could be processed, attempt to demangle the symbol
			if (mangledName && offsetBegin && offsetEnd && mangledName < offsetBegin)
			{
				*mangledName++ = '\0';
				*offsetBegin++ = '\0';
				*offsetEnd++   = '\0';
				*addressBegin++   = '\0';
				*addressEnd++     = '\0';

				int demangleOk;
				char* realName = abi::__cxa_demangle(mangledName, 0, 0, &demangleOk);
				char line[256];
				memset(line, 0, sizeof(line));

				RIO_LOGE("\t[%2d] %s: (%s)+%s in %s"
					, i
					, message
					, (demangleOk == 0 ? realName : mangledName)
					, offsetBegin
					, addr2line(addressBegin, line, sizeof(line))
					);

				free(realName);
			}
			// otherwise, print the whole line
			else
			{
				RIO_LOGE("\t[%2d] %s", i, message);
			}
		}
		free(messages);
	}
} // namespace ErrorFn

} // namespace Rio

#endif // RIO_PLATFORM_LINUX && RIO_COMPILER_GCC
// Copyright (c) 2016 Volodymyr Syvochka