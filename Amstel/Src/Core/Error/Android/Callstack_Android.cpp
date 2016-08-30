// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_ANDROID

#include "Device/Log.h"

namespace Rio
{

namespace ErrorFn
{
	void printCallstack()
	{
		RIO_LOGE("\nCallstack is not supported on Android platform");
	}
} // namespace ErrorFn

} // namespace Rio

#endif // RIO_PLATFORM_ANDROID
// Copyright (c) 2016 Volodymyr Syvochka