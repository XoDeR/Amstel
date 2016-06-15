// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Base/Murmur.h"

namespace Rio
{

// MurmurHash2, by Austin Appleby
// This code makes a few assumptions about how your machine behaves:
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
// And it has a few limitations:
// 1. It will not work incrementally
// 2. It will not produce the same results on little-endian and big-endian machines
uint32_t getMurmurHash32(const void* key, uint32_t length, uint32_t seed)
{
	// 'm' and 'r' are mixing constants generated offline
	// They're not really 'magic', they just happen to work well
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value
	unsigned int h = seed ^ length;

	// Mix 4 bytes at a time into the hash
	const unsigned char * data = (const unsigned char *)key;

	while (length >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		length -= 4;
	}

	// Handle the last few bytes of the input array
	switch (length)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

uint64_t getMurmurHash64(const void* key, uint32_t length, uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995ull;
	const int r = 47;

	uint64_t h = seed ^ (length * m);

	const uint64_t* data = (const uint64_t *)key;
	const uint64_t* end = data + (length / 8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char* data2 = (const unsigned char*)data;

	switch(length & 7)
	{
		case 7: h ^= uint64_t(data2[6]) << 48;
		case 6: h ^= uint64_t(data2[5]) << 40;
		case 5: h ^= uint64_t(data2[4]) << 32;
		case 4: h ^= uint64_t(data2[3]) << 24;
		case 3: h ^= uint64_t(data2[2]) << 16;
		case 2: h ^= uint64_t(data2[1]) << 8;
		case 1: h ^= uint64_t(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka