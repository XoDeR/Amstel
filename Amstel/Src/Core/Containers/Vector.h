// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Memory/Allocator.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/Error/Error.h"

namespace Rio
{

namespace VectorFn
{
	template <typename T> bool getIsEmpty(const Vector<T>& v);
	template <typename T> uint32_t getCount(const Vector<T>& v);
	template <typename T> uint32_t getCapacity(const Vector<T>& v);

	// Resizes the vector to the given size
	// Old items will be copied to the newly created vector
	// If the new capacity is smaller than the previous one, the vector will be truncated
	template <typename T> void resize(Vector<T>& v, uint32_t size);
	// Reserves space in the vector for at least <capacity> items
	template <typename T> void reserve(Vector<T>& v, uint32_t capacity);
	template <typename T> void setCapacity(Vector<T>& v, uint32_t capacity);
	// Grows the vector <v> to contain at least <minCapacity> items
	template <typename T> void grow(Vector<T>& v, uint32_t minCapacity);

	// Condenses the vector <v> so that its capacity matches the actual number of items in the vector
	template <typename T> void condense(Vector<T>& v);
	template <typename T> uint32_t pushBack(Vector<T>& v, const T& item);
	template <typename T> void popBack(Vector<T>& v);

	// Appends <count> items to the vector <v> and returns the number
	// of items in the vector after the append operation
	template <typename T> uint32_t push(Vector<T>& v, const T* items, uint32_t count);

	// Clears the content of the vector <v>
	// Calls destructor on the items
	template <typename T> void clear(Vector<T>& v);

	template <typename T> T* begin(Vector<T>& v);
	template <typename T> const T* begin(const Vector<T>& v);
	template <typename T> T* end(Vector<T>& v);
	template <typename T> const T* end(const Vector<T>& v);

	template <typename T> T& front(Vector<T>& v);
	template <typename T> const T& front(const Vector<T>& v);
	template <typename T> T& back(Vector<T>& v);
	template <typename T> const T& back(const Vector<T>& v);
} // namespace VectorFn

/////////////////
// Implementation
/////////////////

namespace VectorFn
{
	template <typename T>
	inline bool getIsEmpty(const Vector<T>& v)
	{
		return v.size == 0;
	}

	template <typename T>
	inline uint32_t getCount(const Vector<T>& v)
	{
		return v.size;
	}

	template <typename T>
	inline uint32_t getCapacity(const Vector<T>& v)
	{
		return v.capacity;
	}

	template <typename T>
	inline void resize(Vector<T>& v, uint32_t size)
	{
		if (size > v.capacity)
		{
			setCapacity(v, size);
		}

		v.size = size;
	}

	template <typename T>
	inline void reserve(Vector<T>& v, uint32_t capacity)
	{
		if (capacity > v.capacity)
		{
			grow(v, capacity);
		}
	}

	template <typename T>
	inline void setCapacity(Vector<T>& v, uint32_t capacity)
	{
		if (capacity == v.capacity)
		{
			return;
		}

		if (capacity < v.size)
		{
			resize(v, capacity);
		}

		if (capacity > 0)
		{
			T* tmp = v.data;
			v.capacity = capacity;
			v.data = (T*)v.allocator->allocate(capacity * sizeof(T), alignof(T));

			for (uint32_t i = 0; i < v.size; ++i)
			{
				new (v.data + i) T(tmp[i]);
			}

			for (uint32_t i = 0; i < v.size; ++i)
			{
				tmp[i].~T();
			}

			v.allocator->deallocate(tmp);
		}
	}

	template <typename T>
	inline void grow(Vector<T>& v, uint32_t minCapacity)
	{
		uint32_t newCapacity = v.capacity * 2 + 1;

		if (newCapacity < minCapacity)
		{
			newCapacity = minCapacity;
		}

		setCapacity(v, newCapacity);
	}

	template <typename T>
	inline void condense(Vector<T>& v)
	{
		resize(v, v.size);
	}

	template <typename T>
	inline uint32_t pushBack(Vector<T>& v, const T& item)
	{
		if (v.capacity == v.size)
		{
			grow(v, 0);
		}

		construct<T>(v.data + v.size, *v.allocator, IS_ALLOCATOR_AWARE_TYPE(T)());
		v.data[v.size] = item;

		return v.size++;
	}

	template <typename T>
	inline void popBack(Vector<T>& v)
	{
		RIO_ASSERT(v.size > 0, "The vector is empty");
		v.data[v.size - 1].~T();
		--v.size;
	}

	template <typename T>
	inline uint32_t push(Vector<T>& v, const T* items, uint32_t count)
	{
		if (v.capacity <= v.size + count)
		{
			grow(v, v.size + count);
		}

		T* arr = &v.data[v.size];
		for (uint32_t i = 0; i < count; ++i)
		{
			arr[i] = items[i];
		}

		v.size += count;
		return v.size;
	}

	template <typename T>
	inline void clear(Vector<T>& v)
	{
		for (uint32_t i = 0; i < v.size; ++i)
		{
			v.data[i].~T();
		}

		v.size = 0;
	}

	template <typename T>
	inline T* begin(Vector<T>& v)
	{
		return v.data;
	}

	template <typename T>
	inline const T* begin(const Vector<T>& v)
	{
		return v.data;
	}

	template <typename T>
	inline T* end(Vector<T>& v)
	{
		return v.data + v.size;
	}

	template <typename T>
	inline const T* end(const Vector<T>& v)
	{
		return v.data + v.size;
	}

	template <typename T>
	inline T& front(Vector<T>& v)
	{
		RIO_ASSERT(v.size > 0, "The vector is empty");
		return v.data[0];
	}

	template <typename T>
	inline const T& front(const Vector<T>& v)
	{
		RIO_ASSERT(v.size > 0, "The vector is empty");
		return v.data[0];
	}

	template <typename T>
	inline T& back(Vector<T>& v)
	{
		RIO_ASSERT(v.size > 0, "The vector is empty");
		return v.data[v.size - 1];
	}

	template <typename T>
	inline const T& back(const Vector<T>& v)
	{
		RIO_ASSERT(v.size > 0, "The vector is empty");
		return v.data[v.size - 1];
	}
} // namespace VectorFn

template <typename T>
inline Vector<T>::Vector(Allocator& a)
	: allocator(&a)
{
}

template <typename T>
inline Vector<T>::Vector(Allocator& a, uint32_t capacity)
	: allocator(&a)
{
	VectorFn::resize(*this, capacity);
}

template <typename T>
inline Vector<T>::Vector(const Vector<T>& other)
	: allocator(other.allocator)
{
	const uint32_t size = VectorFn::getCount(other);
	VectorFn::resize(*this, size);

	for (uint32_t i = 0; i < size; ++i)
	{
		new (&data[i]) T(other.data[i]);
	}
}

template <typename T>
inline Vector<T>::~Vector()
{
	for (uint32_t i = 0; i < size; ++i)
	{
		data[i].~T();
	}

	allocator->deallocate(data);
}

template <typename T>
inline T& Vector<T>::operator[](uint32_t index)
{
	RIO_ASSERT(index < size, "Index out of bounds");
	return data[index];
}

template <typename T>
inline const T& Vector<T>::operator[](uint32_t index) const
{
	RIO_ASSERT(index < size, "Index out of bounds");
	return data[index];
}

template <typename T>
inline const Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	const uint32_t size = VectorFn::getCount(other);
	VectorFn::resize(*this, size);

	for (uint32_t i = 0; i < size; ++i)
	{
		new (&data[i]) T(other.data[i]);
	}

	return *this;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka