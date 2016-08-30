// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"

#include <algorithm> // std::swap
#include <new>
#include <string.h>  // memcpy

namespace Rio
{

namespace HashMapFn
{
	template <typename TKey, typename TValue, typename Hash> uint32_t getCount(const HashMap<TKey, TValue, Hash>& m);
	// Returns whether the given <key> exists in the map
	template <typename TKey, typename TValue, typename Hash> bool has(const HashMap<TKey, TValue, Hash>& m, const TKey& key);
	// Returns the value for the given <key> or deffault if the key does not exist in the map
	template <typename TKey, typename TValue, typename Hash> const TValue& get(const HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& deffault);
	// Sets the <value> for the <key> in the map
	template <typename TKey, typename TValue, typename Hash> void set(HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& value);
	// Removes the <key> from the map if it exists
	template <typename TKey, typename TValue, typename Hash> void remove(HashMap<TKey, TValue, Hash>& m, const TKey& key);
	// Removes all the items in the map
	// Calls destructor on the items
	template <typename TKey, typename TValue, typename Hash> void clear(HashMap<TKey, TValue, Hash>& m);
} // namespace HashMapFn

namespace HashMapInternalFn
{
	const uint32_t END_OF_LIST = 0xffffffffu;
	const uint32_t DELETED = 0x80000000u;
	const uint32_t FREE = 0x00000000u;

	template <typename TKey, class Hash>
	inline uint32_t getHashKey(const TKey& key)
	{
		const Hash hash;
		uint32_t h = hash(key);

		// the most significant byte is used to indicate a deleted element, so clear it
		h &= 0x7fffffffu;

		// Ensure that we never return 0 as a hash,
		// since we use 0 to indicate that the elem has never been used at all
		h |= h == 0u;

		return h;
	}

	inline bool getIsDeleted(uint32_t hash)
	{
		// the most significant byte set indicates that this hash is a "tombstone"
		return (hash >> 31) != 0;
	}

	template <typename TKey, typename TValue, typename Hash>
	inline uint32_t getProbeDistance(const HashMap<TKey, TValue, Hash>& m, uint32_t hash, uint32_t slotIndex)
	{
		const uint32_t hashIndex = hash & m.mask;
		return (slotIndex + m.capacity - hashIndex) & m.mask;
	}

	template <typename TKey, typename TValue, typename Hash>
	uint32_t find(const HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		if (m.size == 0)
		{
			return END_OF_LIST;
		}

		const uint32_t hash = getHashKey<TKey, Hash>(key);
		uint32_t hashIndex = hash & m.mask;
		uint32_t dist = 0;
		for(;;)
		{
			if (m.hashList[hashIndex] == 0)
			{
				return END_OF_LIST;
			}
			else if (dist > getProbeDistance(m, m.hashList[hashIndex], hashIndex))
			{
				return END_OF_LIST;
			}
			else if (m.hashList[hashIndex] == hash && m.data[hashIndex].pair.first == key)
			{
				return hashIndex;
			}

			hashIndex = (hashIndex + 1) & m.mask;
			++dist;
		}
	}

	template <typename TKey, typename TValue, typename Hash>
	void insert(HashMap<TKey, TValue, Hash>& m, uint32_t hash, TKey& key, TValue& value)
	{
		// goto is used for efficiency there
		uint32_t hashIndex = hash & m.mask;
		uint32_t distance = 0;
		for(;;)
		{
			if (m.hashList[hashIndex] == FREE)
			{
				goto INSERT_AND_RETURN;
			}

			// If the existing element has probed less than us, then swap places with existing element, 
			// and keep going to find another slot for that element
			uint32_t existingElementProbeDistance = getProbeDistance(m, m.hashList[hashIndex], hashIndex);
			if (existingElementProbeDistance < distance)
			{
				if (getIsDeleted(m.hashList[hashIndex]))
				{
					goto INSERT_AND_RETURN;
				}
				std::swap(hash, m.hashList[hashIndex]);
				std::swap(key, m.data[hashIndex].pair.first);
				std::swap(value, m.data[hashIndex].pair.second);
				distance = existingElementProbeDistance;
			}

			hashIndex = (hashIndex + 1) & m.mask;
			++distance;
		}

	INSERT_AND_RETURN:
		new (m.data + hashIndex) typename HashMap<TKey, TValue, Hash>::Entry(*m.allocator);
		m.data[hashIndex].pair.first = key;
		m.data[hashIndex].pair.second = value;
		m.hashList[hashIndex] = hash;
	}

	template <typename TKey, typename TValue, typename Hash>
	void rehash(HashMap<TKey, TValue, Hash>& m, uint32_t newCapacity)
	{
		typedef typename HashMap<TKey, TValue, Hash>::Entry Entry;

		HashMap<TKey, TValue, Hash> newHashMap(*m.allocator);
		newHashMap.hashList = (uint32_t*)newHashMap.allocator->allocate(newCapacity*sizeof(uint32_t), alignof(uint32_t));
		newHashMap.data = (Entry*)newHashMap.allocator->allocate(newCapacity*sizeof(Entry), alignof(Entry));

		// Flag all elements as free
		for (uint32_t i = 0; i < newCapacity; ++i)
		{
			newHashMap.hashList[i] = FREE;
		}

		newHashMap.capacity = newCapacity;
		newHashMap.size = m.size;
		newHashMap.mask = newCapacity - 1;

		for (uint32_t i = 0; i < m.capacity; ++i)
		{
			typename HashMap<TKey, TValue, Hash>::Entry& e = m.data[i];
			const uint32_t hash = m.hashList[i];

			if (hash != FREE && !getIsDeleted(hash))
			{
				HashMapInternalFn::insert(newHashMap, hash, e.pair.first, e.pair.second);
			}
		}

		HashMap<TKey, TValue, Hash> empty(*m.allocator);
		m.~HashMap<TKey, TValue, Hash>();
		memcpy(&m, &newHashMap, sizeof(HashMap<TKey, TValue, Hash>));
		memcpy(&newHashMap, &empty, sizeof(HashMap<TKey, TValue, Hash>));
	}

	template <typename TKey, typename TValue, typename Hash>
	void grow(HashMap<TKey, TValue, Hash>& m)
	{
		const uint32_t newCapacity = (m.capacity == 0 ? 16 : m.capacity * 2);
		rehash(m, newCapacity);
	}

	template <typename TKey, typename TValue, typename Hash>
	bool isFull(const HashMap<TKey, TValue, Hash>& m)
	{
		return m.size >= m.capacity * 0.9f;
	}
} // namespace HashMapInternalFn

namespace HashMapFn
{
	template <typename TKey, typename TValue, typename Hash>
	uint32_t getCount(const HashMap<TKey, TValue, Hash>& m)
	{
		return m.size;
	}

	template <typename TKey, typename TValue, typename Hash>
	bool has(const HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		return HashMapInternalFn::find(m, key) != HashMapInternalFn::END_OF_LIST;
	}

	template <typename TKey, typename TValue, typename Hash>
	const TValue& get(const HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& deffault)
	{
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			return deffault;
		}
		else
		{
			return m.data[i].pair.second;
		}
	}

	template <typename TKey, typename TValue, typename Hash>
	void set(HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& value)
	{
		if (m.size == 0)
		{
			HashMapInternalFn::grow(m);
		}

		// Find or make
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			HashMapInternalFn::insert(m, HashMapInternalFn::getHashKey<TKey, Hash>(key), const_cast<TKey&>(key), const_cast<TValue&>(value));
			++m.size;
		}
		else
		{
			m.data[i].pair.second = value;
		}
		if (HashMapInternalFn::isFull(m) == true)
		{
			HashMapInternalFn::grow(m);
		}
	}

	template <typename TKey, typename TValue, typename Hash>
	void remove(HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			return;
		}

		m.data[i].~Entry();
		m.hashList[i] |= HashMapInternalFn::DELETED;
		--m.size;
	}

	template <typename TKey, typename TValue, typename Hash>
	void clear(HashMap<TKey, TValue, Hash>& m)
	{
		m.size = 0;

		// Flag all elements as free
		for (uint32_t i = 0; i < m.capacity; ++i)
		{
			m.hashList[i] = HashMapInternalFn::FREE;
		}

		for (uint32_t i = 0; i < m.size; ++i)
		{
			m.data[i].~Entry();
		}
	}
} // namespace HashMapFn

template <typename TKey, typename TValue, typename Hash>
HashMap<TKey, TValue, Hash>::HashMap(Allocator& a)
	: allocator(&a)
{
}

template <typename TKey, typename TValue, typename Hash>
HashMap<TKey, TValue, Hash>::~HashMap()
{
	allocator->deallocate(hashList);
	for (uint32_t i = 0; i < size; ++i)
	{
		data[i].~Entry();
	}
	allocator->deallocate(this->data);
}

template <typename TKey, typename TValue, typename Hash>
const TValue& HashMap<TKey, TValue, Hash>::operator[](const TKey& key) const
{
	return HashMapFn::get(*this, key, TValue());
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka