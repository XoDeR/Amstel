// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Functional.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Containers/Pair.h"
#include "Core/Base/Types.h"

namespace Rio
{

// Dynamic array; POD items
// Does not call constructors/destructors; uses memcpy
template <typename T>
struct Array
{
	ALLOCATOR_AWARE;

	Array(Allocator& a);
	Array(Allocator& a, uint32_t capacity);
	Array(const Array<T>& other);
	~Array();
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
	Array<T>& operator=(const Array<T>& other);

	Allocator* allocator;
	uint32_t capacity = 0;
	uint32_t size = 0;
	T* data = nullptr;
};

using Buffer = Array<char>;

// Dynamic array; non-POD items
// Calls constructors and destructors
// If your items are POD, use Array<T> instead
template <typename T>
struct Vector
{
	ALLOCATOR_AWARE;

	Vector(Allocator& a);
	Vector(Allocator& a, uint32_t capacity);
	Vector(const Vector<T>& other);
	~Vector();
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
	const Vector<T>& operator=(const Vector<T>& other);

	Allocator* allocator;
	uint32_t capacity = 0;
	uint32_t size = 0;
	T* data = nullptr;
};

// Circular buffer double-ended queue of POD items
template <typename T>
struct Queue
{
	ALLOCATOR_AWARE;

	uint32_t read = 0;
	uint32_t size = 0;
	Array<T> queue;

	Queue(Allocator& a);
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
};

// Priority queue of POD items
template <typename T>
struct PriorityQueue
{
	ALLOCATOR_AWARE;
	Array<T> queue;
	PriorityQueue(Allocator& a);
};

// Map from key to value
// Uses a Vector internally
// Not suited to performance-critical stuff
template <typename TKey, typename TValue>
struct Map
{
	ALLOCATOR_AWARE;

	struct Node
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;
		uint32_t left;
		uint32_t right;
		uint32_t parent;
		uint32_t color;

		Node(Allocator& a)
			: pair(a)
		{
		}
	};

	Map(Allocator& a);
	const TValue& operator[](const TKey& key) const;

	uint32_t root;
	uint32_t sentinel;
	Vector<Node> data;
};

template <typename TKey, typename TValue, class Hash = THash<TKey> >
struct HashMap
{
	ALLOCATOR_AWARE;

	struct Entry
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;

		Entry(Allocator& a)
			: pair(a)
		{
		}
	};

	HashMap(Allocator& a);
	~HashMap();
	const TValue& operator[](const TKey& key) const;

	Allocator* allocator;
	uint32_t capacity = 0;
	uint32_t size = 0;
	uint32_t mask = 0;
	uint32_t* hashList = nullptr;
	Entry* data = nullptr;
};

// Vector of sorted items
// Items are not automatically sorted, you have to call SortMapFn::sort()
// whenever you are done inserting/removing items
template <typename TKey, typename TValue, class Compare = Less<TKey> >
struct SortMap
{
	ALLOCATOR_AWARE;

	struct Entry
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;

		Entry(Allocator& a)
			: pair(a)
		{
		}
	};

	SortMap(Allocator& a);

	Vector<Entry> data;
#if RIO_DEBUG
	bool isSorted;
#endif
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka