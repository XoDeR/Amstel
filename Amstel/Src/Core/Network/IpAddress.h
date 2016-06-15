// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"

namespace Rio
{

struct IpAddress
{
	uint32_t ipAddress;

	// Default is 127.0.0.1
	IpAddress()
	{
		set(127, 0, 0, 1);
	}

	IpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
	{
		set(a, b, c, d);
	}

	// Returns the IP address as packed unsigned 32-bit integer
	uint32_t getAddress() const
	{
		return ipAddress;
	}

	void set(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
	{
		ipAddress = 0;
		ipAddress |= uint32_t(a) << 24;
		ipAddress |= uint32_t(b) << 16;
		ipAddress |= uint32_t(c) << 8;
		ipAddress |= uint32_t(d) << 0;
	}
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka