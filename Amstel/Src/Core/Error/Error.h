// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Config.h"

namespace Rio
{

namespace ErrorFn
{
	// Aborts the program execution logging an error message and the stacktrace if the platform supports it
	void abort(const char* file, int line, const char* format, ...);
	void printCallstack();
} // namespace ErrorFn

} // namespace Rio

#if RIO_DEBUG
	#define RIO_ASSERT(condition, message, ...) do { if (!(condition)) {\
		Rio::ErrorFn::abort(__FILE__, __LINE__, "\nAssertion failed: %s\n\t" message "\n", #condition, ##__VA_ARGS__); }} while (0)
#else
	#define RIO_ASSERT(...) ((void)0)
#endif // RIO_DEBUG

#define RIO_ASSERT_NOT_NULL(pointer) RIO_ASSERT(pointer != nullptr, #pointer " must be not null")
#define RIO_FATAL(message) RIO_ASSERT(false, message)
#define RIO_ENSURE(condition) RIO_ASSERT(condition, "")
// Copyright (c) 2016 Volodymyr Syvochka