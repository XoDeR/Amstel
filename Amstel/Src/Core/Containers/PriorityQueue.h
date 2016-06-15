#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Containers/Array.h"

#include <algorithm>

namespace Rio
{

namespace PriorityQueueFn
{
	// Returns the first item in the queue
	template <typename T> const T& getTop(const PriorityQueue<T>& q);
	// Inserts <item> into the queue
	template <typename T> void push(PriorityQueue<T>& q, const T& item);
	// Removes the first item from the queue
	template <typename T> void pop(PriorityQueue<T>& q);
} // namespace PriorityQueueFn

namespace PriorityQueueFn
{
	template <typename T>
	const T& getTop(const PriorityQueue<T>& q)
	{
		return ArrayFn::front(q.queue);
	}

	template <typename T>
	void push(PriorityQueue<T>& q, const T& item)
	{
		ArrayFn::pushBack(q.queue, item);
		std::push_heap(ArrayFn::begin(q.queue), ArrayFn::end(q.queue));
	}

	template <typename T>
	void pop(PriorityQueue<T>& q)
	{
		std::pop_heap(ArrayFn::begin(q.queue), ArrayFn::end(q.queue));
		ArrayFn::popBack(q.queue);
	}
} // namespace PriorityQueueFn

template <typename T>
PriorityQueue<T>::PriorityQueue(Allocator& a)
	: queue(a)
{
}

} // namespace Rio
