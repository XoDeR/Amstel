// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Strings/DynamicString.h"
#include "Core/Base/Guid.h"
#include "Core/Base/Macros.h"
#include "Core/Base/Platform.h"

#if RIO_PLATFORM_POSIX
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#elif RIO_PLATFORM_WINDOWS
	#include <objbase.h>
#endif // RIO_PLATFORM_POSIX

namespace Rio
{

namespace GuidFn
{
	Guid createGuid()
	{
		Guid guid;
#if RIO_PLATFORM_POSIX
		int fd = open("/dev/urandom", O_RDONLY);
		RIO_ASSERT(fd != -1, "open: erron = %d", errno);
		read(fd, &guid, sizeof(guid));
		close(fd);
		guid.data3 = (guid.data3 & 0x4fffu) | 0x4000u;
		guid.data4 = (guid.data4 & 0x3fffffffffffffffu) | 0x8000000000000000u;
#elif RIO_PLATFORM_WINDOWS
		HRESULT hr = CoCreateGuid((GUID*)&guid);
		RIO_ASSERT(hr == S_OK, "CoCreateGuid: error");
		RIO_UNUSED(hr);
#endif // RIO_PLATFORM_POSIX
		return guid;
	}

	Guid parse(const char* str)
	{
		Guid guid;
		tryParse(str, guid);
		return guid;
	}

	bool tryParse(const char* str, Guid& guid)
	{
		RIO_ASSERT_NOT_NULL(str);
		uint32_t a;
		uint32_t b;
		uint32_t c;
		uint32_t d;
		uint32_t e;
		uint32_t f;
		int tokenCount = sscanf(str, "%8x-%4x-%4x-%4x-%4x%8x", &a, &b, &c, &d, &e, &f);
		guid.data1 = a;
		guid.data2 = (uint16_t)(b & 0x0000ffffu);
		guid.data3 = (uint16_t)(c & 0x0000ffffu);
		guid.data4 = (uint64_t)(d & 0x0000ffffu) << 48 | (uint64_t)(e & 0x0000ffffu) << 32 | (uint64_t)f;
		return tokenCount == 6;
	}

	void toString(const Guid& guid, DynamicString& s)
	{
		char str[36+1];
		snPrintF(str, sizeof(str), "%.8x-%.4x-%.4x-%.4x-%.4x%.8x"
			, guid.data1
			, guid.data2
			, guid.data3
			, (uint16_t)((guid.data4 & 0xffff000000000000u) >> 48)
			, (uint16_t)((guid.data4 & 0x0000ffff00000000u) >> 32)
			, (uint32_t)((guid.data4 & 0x00000000ffffffffu) >>  0)
			);
		s.set(str, sizeof(str)-1);
	}
} // namespace GuidFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka