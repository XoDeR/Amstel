// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/Log.h"
#include "Device/Device.h"
#include "Core/Thread/Mutex.h"
#include "Core/Base/Os.h"
#include "Core/Base/Platform.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

namespace LogInternalFn
{
	static Mutex mutex;

	void logEx(LogSeverity::Enum sev, const char* msg, va_list args)
	{
		ScopedMutex sm(mutex);

		char buf[8192];
		int length = vsnPrintF(buf, sizeof(buf), msg, args);
		buf[length] = '\0';

#if RIO_PLATFORM_POSIX
		#define ANSI_RESET  "\x1b[0m"
		#define ANSI_YELLOW "\x1b[33m"
		#define ANSI_RED    "\x1b[31m"

		static const char* stt[] =
		{
			ANSI_RESET,
			ANSI_YELLOW,
			ANSI_RED,
			ANSI_RESET
		};

		OsFn::log(stt[sev]);
		OsFn::log(buf);
		OsFn::log(ANSI_RESET);
#else
		OsFn::log(buf);
#endif // RIO_PLATFORM_POSIX
		OsFn::log("\n");

		if (getDevice())
		{
			getDevice()->log(buf, sev);
		}
	}

	void logEx(LogSeverity::Enum sev, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		logEx(sev, msg, args);
		va_end(args);
	}
} // namespace LogInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka