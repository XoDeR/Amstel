// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/Vector.h"

#include <algorithm>

namespace Rio
{

namespace SortMapFn
{
	template <typename TKey, typename TValue, typename Compare> uint32_t getCount(const SortMap<TKey, TValue, Compare>& m);
	// Returns whether the <key> exists in the map
	template <typename TKey, typename TValue, typename Compare> bool has(const SortMap<TKey, TValue, Compare>& m, const TKey& key);
	// Returns the value for the given <key> or <deffault> if the key does not exist in the map
	template <typename TKey, typename TValue, typename Compare> const TValue& get(const SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault);
	// Returns the value for the given <key> or <deffault> if the key does not exist in the map
	template <typename TKey, typename TValue, typename Compare> TValue& get(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault);
	template <typename TKey, typename TValue, typename Compare> void sort(SortMap<TKey, TValue, Compare>& m);
	template <typename TKey, typename TValue, typename Compare> void set(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& val);
	// Removes the key from the map if it exists
	template <typename TKey, typename TValue, typename Compare> void remove(SortMap<TKey, TValue, Compare>& m, const TKey& key);
	// Removes all the items in the map
	template <typename TKey, typename TValue, typename Compare> void clear(SortMap<TKey, TValue, Compare>& m);
	// Returns a pointer to the first item in the map, can be used to efficiently iterate over the elements
	template <typename TKey, typename TValue, typename Compare> const typename SortMap<TKey, TValue, Compare>::Entry* begin(const SortMap<TKey, TValue, Compare>& m);
	template <typename TKey, typename TValue, typename Compare> const typename SortMap<TKey, TValue, Compare>::Entry* end(const SortMap<TKey, TValue, Compare>& m);
} // namespace SortMapFn

namespace SortMapInternalFn
{
	const uint32_t END_OF_LIST = 0xffffffffu;

	struct FindResult
	{
		uint32_t itemIndex;
	};

	template <typename TKey, typename TValue, typename Compare>
	struct CompareEntry
	{
		bool operator()(const typename SortMap<TKey, TValue, Compare>::Entry& a, const typename SortMap<TKey, TValue, Compare>::Entry& b) const
		{
			return comp(a.pair.first, b.pair.first);
		}

		bool operator()(const typename SortMap<TKey, TValue, Compare>::Entry& a, const TKey& key) const
		{
			return comp(a.pair.first, key);
		}

		Compare comp;
	};

	template <typename TKey, typename TValue, typename Compare>
	inline FindResult find(const SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		RIO_ASSERT(m.isSorted, "Map not sorted");

		FindResult result;
		result.itemIndex = END_OF_LIST;

		const typename SortMap<TKey, TValue, Compare>::Entry* first =
			std::lower_bound(VectorFn::begin(m.data), VectorFn::end(m.data), key,
				SortMapInternalFn::CompareEntry<TKey, TValue, Compare>());

		if (first != VectorFn::end(m.data) && !(key < first->pair.first))
		{
			result.itemIndex = uint32_t(first - VectorFn::begin(m.data));
		}

		return result;
	}
} // namespace SortMapInternalFn

namespace SortMapFn
{
	template <typename TKey, typename TValue, typename Compare>
	inline uint32_t getCount(const SortMap<TKey, TValue, Compare>& m)
	{
		return VectorFn::getCount(m.data);
	}

	template <typename TKey, typename TValue, typename Compare>
	inline bool has(const SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		return SortMapInternalFn::find(m, key).itemIndex != SortMapInternalFn::END_OF_LIST;
	}

	template <typename TKey, typename TValue, typename Compare>
	const TValue& get(const SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault)
	{
		SortMapInternalFn::FindResult result = SortMapInternalFn::find(m, key);

		if (result.itemIndex == SortMapInternalFn::END_OF_LIST)
		{
			return deffault;
		}

		return m.data[result.itemIndex].pair.second;
	}

	template <typename TKey, typename TValue, typename Compare>
	TValue& get(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault)
	{
		return const_cast<TValue&>(get(static_cast<const SortMap<TKey, TValue, Compare>&>(m), key, deffault));
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void sort(SortMap<TKey, TValue, Compare>& m)
	{
		std::sort(VectorFn::begin(m.data), VectorFn::end(m.data), SortMapInternalFn::CompareEntry<TKey, TValue, Compare>());
#if RIO_DEBUG
		m.isSorted = true;
#endif // RIO_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void set(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& val)
	{
		SortMapInternalFn::FindResult result = SortMapInternalFn::find(m, key);

		if (result.itemIndex == SortMapInternalFn::END_OF_LIST)
		{
			typename SortMap<TKey, TValue, Compare>::Entry e(*m.data.allocator);
			e.pair.first = key;
			e.pair.second = val;
			VectorFn::pushBack(m.data, e);
		}
		else
		{
			m.data[result.itemIndex].pair.second = val;
		}
#if RIO_DEBUG
		m.isSorted = false;
#endif // RIO_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void remove(SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		SortMapInternalFn::FindResult result = SortMapInternalFn::find(m, key);

		if (result.itemIndex == SortMapInternalFn::END_OF_LIST)
		{
			return;
		}

		if (VectorFn::getCount(m.data))
		{
			m.data[result.itemIndex] = m.data[VectorFn::getCount(m.data) - 1];
			VectorFn::popBack(m.data);
		}
#if RIO_DEBUG
		m.isSorted = false;
#endif // RIO_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void clear(SortMap<TKey, TValue, Compare>& m)
	{
		VectorFn::clear(m.data);
#if RIO_DEBUG
		m.isSorted = true;
#endif // RIO_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* begin(const SortMap<TKey, TValue, Compare>& m)
	{
		return VectorFn::begin(m.data);
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* end(const SortMap<TKey, TValue, Compare>& m)
	{
		return VectorFn::end(m.data);
	}
} // namespace SortMapFn

template <typename TKey, typename TValue, typename Compare>
inline SortMap<TKey, TValue, Compare>::SortMap(Allocator& a)
	: data(a)
#if RIO_DEBUG
	, isSorted(true)
#endif // RIO_DEBUG
{
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka