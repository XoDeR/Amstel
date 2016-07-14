// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Platform.h"

#define RIO_VERSION "0.0.0"

#ifndef RIO_DEBUG
	#define RIO_DEBUG 0
#endif // RIO_DEBUG

#ifndef RIO_DEVELOPMENT
	#define RIO_DEVELOPMENT 1
#endif // RIO_DEVELOPMENT

#define RIO_RELEASE (!RIO_DEBUG && !RIO_DEVELOPMENT)

#ifndef RIO_BUILD_UNIT_TESTS
	#define RIO_BUILD_UNIT_TESTS 1
#endif // RIO_BUILD_UNIT_TESTS

#if !defined(RIO_PHYSICS_BULLET) \
	&& !defined(RIO_PHYSICS_PHYSX) \
	&& !defined(RIO_PHYSICS_NULL)

	#ifndef RIO_PHYSICS_PHYSX
		#define RIO_PHYSICS_PHYSX 0
	#endif // RIO_PHYSICS_PHYSX

	#ifndef RIO_PHYSICS_BULLET
		#define RIO_PHYSICS_BULLET 0
	#endif // RIO_PHYSICS_BULLET

	#ifndef RIO_PHYSICS_NULL
		#define RIO_PHYSICS_NULL 1
	#endif // RIO_PHYSICS_NULL
#else
	#ifndef RIO_PHYSICS_PHYSX
		#define RIO_PHYSICS_PHYSX 0
	#endif // RIO_PHYSICS_PHYSX

	#ifndef RIO_PHYSICS_BULLET
		#define RIO_PHYSICS_BULLET 0
	#endif // RIO_PHYSICS_BULLET

	#ifndef RIO_PHYSICS_NULL
		#define RIO_PHYSICS_NULL 0
	#endif // RIO_PHYSICS_NULL
#endif

#if !defined(RIO_SOUND_OPENAL) \
	&& !defined(RIO_SOUND_NULL)

	#ifndef RIO_SOUND_OPENAL
		#define RIO_SOUND_OPENAL 0
	#endif // RIO_SOUND_OPENAL

	#ifndef RIO_SOUND_NULL
		#define RIO_SOUND_NULL 1
	#endif // RIO_SOUND_NULL
#else
	#ifndef RIO_SOUND_OPENAL
		#define RIO_SOUND_OPENAL 0
	#endif // RIO_SOUND_OPENAL

	#ifndef RIO_SOUND_NULL
		#define RIO_SOUND_NULL 0
	#endif // RIO_SOUND_NULL
#endif

#ifndef RIO_DEFAULT_PIXELS_PER_METER
	#define RIO_DEFAULT_PIXELS_PER_METER 32
#endif // RIO_DEFAULT_PIXELS_PER_METER

#ifndef RIO_DEFAULT_WINDOW_WIDTH
	#define RIO_DEFAULT_WINDOW_WIDTH 1280
#endif // RIO_DEFAULT_WINDOW_WIDTH

#ifndef RIO_DEFAULT_WINDOW_HEIGHT
	#define RIO_DEFAULT_WINDOW_HEIGHT 720
#endif // RIO_DEFAULT_WINDOW_HEIGHT

#ifndef RIO_DEFAULT_CONSOLE_PORT
	#define RIO_DEFAULT_CONSOLE_PORT 10001
#endif // RIO_DEFAULT_CONSOLE_PORT

#ifndef RIO_DEFAULT_COMPILER_PORT
	#define RIO_DEFAULT_COMPILER_PORT 10618
#endif // RIO_DEFAULT_COMPILER_PORT

#ifndef RIO_BOOT_CONFIG
	#define RIO_BOOT_CONFIG "Boot"
#endif // RIO_BOOT_CONFIG

#ifndef RIO_DATA_DIRECTORY
	#define RIO_DATA_DIRECTORY "Data"
#endif // RIO_DATA_DIRECTORY

#ifndef RIO_TEMP_DIRECTORY
	#define RIO_TEMP_DIRECTORY "Temp"
#endif // RIO_TEMP_DIRECTORY

#ifndef RIO_BUNDLEIGNORE
	#define RIO_BUNDLEIGNORE ".bundleIgnore"
#endif // RIO_BUNDLEIGNORE

#ifndef RIO_LAST_LOG
	#define RIO_LAST_LOG "Last.log"
#endif // RIO_LAST_LOG

#ifndef RIO_MAX_JOYPADS
	#define RIO_MAX_JOYPADS 4
#endif // RIO_MAX_JOYPADS

#ifndef RIO_MAX_LUA_VECTOR3
	#define RIO_MAX_LUA_VECTOR3 8192
#endif // RIO_MAX_LUA_VECTOR3

#ifndef RIO_MAX_LUA_MATRIX4X4
	#define RIO_MAX_LUA_MATRIX4X4 8192
#endif // RIO_MAX_LUA_MATRIX4X4

#ifndef RIO_MAX_LUA_QUATERNION
	#define RIO_MAX_LUA_QUATERNION 8192
#endif // RIO_MAX_LUA_MATRIX4X4
// Copyright (c) 2016 Volodymyr Syvochka