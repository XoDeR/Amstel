// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include <stdarg.h>

namespace Rio
{

struct LogSeverity
{
	enum Enum
	{
		INFO,
		WARN,
		ERROR,
		DEBUG,

		COUNT
	};
};

namespace LogInternalFn
{
	void logEx(LogSeverity::Enum sev, const char* msg, va_list args);
	void logEx(LogSeverity::Enum sev, const char* msg, ...);
} // namespace LogInternalFn

} // namespace Rio

#define RIO_LOGIV(msg, va_list) Rio::LogInternalFn::logEx(Rio::LogSeverity::INFO, msg, va_list)
#define RIO_LOGDV(msg, va_list) Rio::LogInternalFn::logEx(Rio::LogSeverity::DEBUG, msg, va_list)
#define RIO_LOGEV(msg, va_list) Rio::LogInternalFn::logEx(Rio::LogSeverity::ERROR, msg, va_list)
#define RIO_LOGWV(msg, va_list) Rio::LogInternalFn::logEx(Rio::LogSeverity::WARN, msg, va_list)
#define RIO_LOGI(msg, ...) Rio::LogInternalFn::logEx(Rio::LogSeverity::INFO, msg, ##__VA_ARGS__)
#define RIO_LOGD(msg, ...) Rio::LogInternalFn::logEx(Rio::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
#define RIO_LOGE(msg, ...) Rio::LogInternalFn::logEx(Rio::LogSeverity::ERROR, msg, ##__VA_ARGS__)
#define RIO_LOGW(msg, ...) Rio::LogInternalFn::logEx(Rio::LogSeverity::WARN, msg, ##__VA_ARGS__)
// Copyright (c) 2016 Volodymyr Syvochka