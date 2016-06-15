// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Error/Error.h"
#include "Device/Log.h"

#include <stdarg.h>
#include <stdlib.h> // exit

namespace Rio
{

namespace ErrorFn
{
	static void abort(const char* file, int line, const char* format, va_list args)
	{
		RIO_LOGEV(format, args);
		RIO_LOGE("\tIn: %s:%d\n\nStacktrace:", file, line);
		printCallstack();
		exit(EXIT_FAILURE);
	}

	void abort(const char* file, int line, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		abort(file, line, format, args);
		va_end(args);
	}
} // namespace ErrorFn

} // namespace Rio
  // Copyright (c) 2016 Volodymyr Syvochka