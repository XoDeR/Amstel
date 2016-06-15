// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/Error/Error.h"

#include <string.h> // memcpy

namespace Rio
{

namespace ArrayFn
{
	// Functionality mimics corresponding STL functions (API)
	template <typename T> bool getIsEmpty(const Array<T>& a);
	template <typename T> uint32_t getCount(const Array<T>& a);
	// Returns the maximum number of items the array a can hold
	template <typename T> uint32_t getCapacity(const Array<T>& a);
	// Resizes the array to the given size
	// Old items will be copied to the newly created array
	// If the new capacity is smaller than the previous one, the array will be truncated
	template <typename T> void resize(Array<T>& a, uint32_t size);
	// Reserves space in the array for at least <capacity> items
	template <typename T> void reserve(Array<T>& a, uint32_t capacity);
	// Sets the capacity of array
	template <typename T> void setCapacity(Array<T>& a, uint32_t capacity);
	// Grows the array to contain at least minCapacity items
	template <typename T> void grow(Array<T>& a, uint32_t minCapacity);
	// Condenses the array so that its capacity matches the actual number of items in the array
	template <typename T> void condense(Array<T>& a);
	// Appends an item to the array and returns its index
	template <typename T> uint32_t pushBack(Array<T>& a, const T& item);
	// Removes the last item from the array
	template <typename T> void popBack(Array<T>& a);
	// Appends <count> items to the array <a> and returns the number
	// of items in the array after the append operation
	template <typename T> uint32_t push(Array<T>& a, const T* items, uint32_t count);

	// Clears the content of the array <a>
	// Does not free memory nor call destructors, it only zeroes
	// the number of items in the array
	template <typename T> void clear(Array<T>& a);

	template <typename T> T* begin(Array<T>& a);
	template <typename T> const T* begin(const Array<T>& a);
	template <typename T> T* end(Array<T>& a);
	template <typename T> const T* end(const Array<T>& a);

	template <typename T> T& front(Array<T>& a);
	template <typename T> const T& front(const Array<T>& a);
	template <typename T> T& back(Array<T>& a);
	template <typename T> const T& back(const Array<T>& a);
} // namespace ArrayFn

namespace ArrayFn
{
	template <typename T>
	inline bool getIsEmpty(const Array<T>& a)
	{
		return a.size == 0;
	}

	template <typename T>
	inline uint32_t getCount(const Array<T>& a)
	{
		return a.size;
	}

	template <typename T>
	inline uint32_t getCapacity(const Array<T>& a)
	{
		return a.capacity;
	}

	template <typename T>
	inline void resize(Array<T>& a, uint32_t size)
	{
		if (size > a.capacity)
		{
			setCapacity(a, size);
		}

		a.size = size;
	}

	template <typename T>
	inline void reserve(Array<T>& a, uint32_t capacity)
	{
		if (capacity > a.capacity)
		{
			grow(a, capacity);
		}
	}

	template <typename T>
	inline void setCapacity(Array<T>& a, uint32_t capacity)
	{
		if (capacity == a.capacity)
		{
			return;
		}

		if (capacity < a.size)
		{
			resize(a, capacity);
		}

		if (capacity > 0)
		{
			T* tmp = a.data;
			a.capacity = capacity;
			a.data = (T*)a.allocator->allocate(capacity * sizeof(T), alignof(T));

			memcpy(a.data, tmp, a.size * sizeof(T));

			a.allocator->deallocate(tmp);
		}
	}

	template <typename T>
	inline void grow(Array<T>& a, uint32_t minCapacity)
	{
		uint32_t newCapacity = a.capacity * 2 + 1;

		if (newCapacity < minCapacity)
		{
			newCapacity = minCapacity;
		}

		setCapacity(a, newCapacity);
	}

	template <typename T>
	inline void condense(Array<T>& a)
	{
		resize(a, a.size);
	}

	template <typename T>
	inline uint32_t pushBack(Array<T>& a, const T& item)
	{
		if (a.capacity == a.size)
		{
			grow(a, 0);
		}

		a.data[a.size] = item;

		return a.size++;
	}

	template <typename T>
	inline void popBack(Array<T>& a)
	{
		RIO_ASSERT(a.size > 0, "The array is empty");
		--a.size;
	}

	template <typename T>
	inline uint32_t push(Array<T>& a, const T* items, uint32_t count)
	{
		if (a.capacity <= a.size + count)
		{
			grow(a, a.size + count);
		}

		memcpy(&a.data[a.size], items, sizeof(T) * count);
		a.size += count;

		return a.size;
	}

	template <typename T>
	inline void clear(Array<T>& a)
	{
		a.size = 0;
	}

	template <typename T>
	inline const T* begin(const Array<T>& a)
	{
		return a.data;
	}

	template <typename T>
	inline T* begin(Array<T>& a)
	{
		return a.data;
	}

	template <typename T>
	inline const T* end(const Array<T>& a)
	{
		return a.data + a.size;
	}

	template <typename T>
	inline T* end(Array<T>& a)
	{
		return a.data + a.size;
	}

	template <typename T>
	inline T& front(Array<T>& a)
	{
		RIO_ASSERT(a.size > 0, "The array is empty");
		return a.data[0];
	}

	template <typename T>
	inline const T& front(const Array<T>& a)
	{
		RIO_ASSERT(a.size > 0, "The array is empty");
		return a.data[0];
	}

	template <typename T>
	inline T& back(Array<T>& a)
	{
		RIO_ASSERT(a.size > 0, "The array is empty");
		return a.data[a.size - 1];
	}

	template <typename T>
	inline const T& back(const Array<T>& a)
	{
		RIO_ASSERT(a.size > 0, "The array is empty");
		return a.data[a.size - 1];
	}
} // namespace ArrayFn

template <typename T>
inline Array<T>::Array(Allocator& a)
	: allocator(&a)
{
}

template <typename T>
inline Array<T>::Array(Allocator& a, uint32_t capacity)
	: allocator(&a)
{
	ArrayFn::resize(*this, capacity);
}

template <typename T>
inline Array<T>::Array(const Array<T>& other)
	: allocator(other.allocator)
{
	const uint32_t size = other.size;
	ArrayFn::resize(*this, size);
	memcpy(data, other.data, sizeof(T) * size);
}

template <typename T>
inline Array<T>::~Array()
{
	allocator->deallocate(data);
}

template <typename T>
inline T& Array<T>::operator[](uint32_t index)
{
	RIO_ASSERT(index < size, "Index out of bounds");
	return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint32_t index) const
{
	RIO_ASSERT(index < size, "Index out of bounds");
	return data[index];
}

template <typename T>
inline Array<T>& Array<T>::operator=(const Array<T>& other)
{
	const uint32_t size = other.size;
	ArrayFn::resize(*this, size);
	memcpy(this->data, other.data, sizeof(T) * size);
	return *this;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka