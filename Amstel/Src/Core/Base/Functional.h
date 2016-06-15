// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Murmur.h"
#include "Core/Base/Types.h"

namespace Rio
{

template<typename T>
struct EqualTo
{
	bool operator()(const T& a, const T& b) const
	{
		return a == b;
	};
};

template<typename T>
struct NotEqualTo
{
	bool operator()(const T& a, const T& b) const
	{
		return a != b;
	};
};

template <typename T>
struct Greater
{
	bool operator()(const T& a, const T& b) const
	{
		return a > b;
	};
};

template<typename T>
struct Less
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
	};
};

template<typename T>
struct GreaterEqual
{
	bool operator()(const T& a, const T& b) const
	{
		return a >= b;
	};
};

template<typename T>
struct LessEqual
{
	bool operator()(const T& a, const T& b) const
	{
		return a <= b;
	};
};

// Hash functions
// THash (template hash) name is selected to distinguish with functions 
// not to cause ambiguity in names
template <typename T>
struct THash;

template<>
struct THash<bool>
{
	uint32_t operator()(const bool val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<int8_t>
{
	uint32_t operator()(const int8_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<uint8_t>
{
	uint32_t operator()(const uint8_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<int16_t>
{
	uint32_t operator()(const int16_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<uint16_t>
{
	uint32_t operator()(const uint16_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<int32_t>
{
	uint32_t operator()(const int32_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<uint32_t>
{
	uint32_t operator()(const uint32_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<int64_t>
{
	uint32_t operator()(const int64_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<uint64_t>
{
	uint32_t operator()(const uint64_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct THash<float>
{
	uint32_t operator()(const float val) const
	{
		return val == 0.0f ? 0 : getMurmurHash32(&val, sizeof(val), 0);
	}
};

template<>
struct THash<double>
{
	uint32_t operator()(const double val) const
	{
		return val == 0.0 ? 0 : getMurmurHash32(&val, sizeof(val), 0);
	}
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka