// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Containers/Vector.h"

// #define RBTREE_VERIFY

namespace Rio
{

namespace MapFn
{
	template <typename TKey, typename TValue> uint32_t getCount(const Map<TKey, TValue>& m);
	// Returns whether the <key> exists in the
	template <typename TKey, typename TValue> bool has(const Map<TKey, TValue>& m, const TKey& key);
	// Returns the value for the <key> or <deffault> if the key does not exist in the map
	template <typename TKey, typename TValue> const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault);
	// Sets the <value> for the <key> in the map
	template <typename TKey, typename TValue> void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value);
	// Removes the <key> from the map if it exists
	template <typename TKey, typename TValue> void remove(Map<TKey, TValue>& m, const TKey& key);
	// Removes all the items in the map
	// Calls destructor on the items
	template <typename TKey, typename TValue> void clear(Map<TKey, TValue>& m);
	// Returns a pointer to the first item in the map, can be used to efficiently iterate over the elements (in random order)
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m);
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m);
} // namespace MapFn

namespace MapInternalFn
{
	const uint32_t BLACK = 0xB1B1B1B1u;
	const uint32_t RED = 0xEDEDEDEDu;
	const uint32_t NIL = 0xFFFFFFFFu;

	template <typename TKey, typename TValue>
	inline uint32_t getRoot(const Map<TKey, TValue>& m)
	{
		return m.root;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getParent(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);
		return m.data[n].parent;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getLeft(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);
		return m.data[n].left;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getRight(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);
		return m.data[n].right;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getColor(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);
		return m.data[n].color;
	}

#ifdef RBTREE_VERIFY
	template<typename TKey, typename TValue>
	inline int32_t debugVerify(Map<TKey, TValue>& m, uint32_t n)
	{
		if (n == m.sentinel)
		{
			return 0;
		}

		if (getLeft(m, n) != m.sentinel)
		{
			RIO_ASSERT(getParent(m, getLeft(m, n)) == n, "Bad RBTree");
			RIO_ASSERT(m.data[getLeft(m, n)].pair.first < m.data[n].pair.first, "Bad RBTree");
		}

		if (getRight(m, n) != m.sentinel)
		{
			RIO_ASSERT(getParent(m, getRight(m, n)) == n, "Bad RBTree");
			RIO_ASSERT(m.data[n].pair.first < m.data[getRight(m, n)].pair.first, "Bad RBTree");
		}

		int32_t left = debugVerify(m, getLeft(m, n));
		int32_t rigth = debugVerify(m, getRight(m, n));
		RIO_ASSERT(left == rigth, "Bad RBTree");

		if (getColor(m, n) == BLACK)
		{
			left += 1;
		}
		else
		{
			if (getParent(m, n) != NIL && getColor(m, getParent(m, n)) == RED)
			{
				RIO_ASSERT(false, "Bad RBTree");
			}
		}

		return left;
	}
#endif // RBTREE_VERIFY

	template <typename TKey, typename TValue>
	inline uint32_t min(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.sentinel)
		{
			return x;
		}

		while (getLeft(m, x) != m.sentinel)
		{
			x = getLeft(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t max(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.sentinel)
		{
			return x;
		}

		while (getRight(m, x) != m.sentinel)
		{
			x = getRight(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getSuccessor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (getRight(m, x) != m.sentinel)
		{
			return min(m, getRight(m, x));
		}

		uint32_t y = getParent(m, x);

		while (y != NIL && x == getRight(m, y))
		{
			x = y;
			y = getParent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getPredecessor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (getLeft(m, x) != m.sentinel)
		{
			return max(m, getLeft(m, x));
		}

		uint32_t y = getParent(m, x);

		while (y != NIL && x == getLeft(m, y))
		{
			x = y;
			y = getParent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline void rotateLeft(Map<TKey, TValue>& m, uint32_t x)
	{
		RIO_ASSERT(x < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), x);

		uint32_t y = getRight(m, x);
		m.data[x].right = getLeft(m, y);

		if (getLeft(m, y) != m.sentinel)
		{
			m.data[getLeft(m, y)].parent = x;
		}

		m.data[y].parent = getParent(m, x);

		if (getParent(m, x) == NIL)
		{
			m.root = y;
		}
		else
		{
			if (x == getLeft(m, getParent(m, x)))
			{
				m.data[getParent(m, x)].left = y;
			}
			else
			{
				m.data[getParent(m, x)].right = y;
			}
		}

		m.data[y].left = x;
		m.data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void rotateRight(Map<TKey, TValue>& m, uint32_t x)
	{
		RIO_ASSERT(x < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), x);

		uint32_t y = getLeft(m, x);
		m.data[x].left = getRight(m, y);

		if (getRight(m, y) != m.sentinel)
		{
			m.data[getRight(m, y)].parent = x;
		}

		m.data[y].parent = getParent(m, x);

		if (getParent(m, x) == NIL)
		{
			m.root = y;
		}
		else
		{
			if (x == getLeft(m, getParent(m, x)))
			{
				m.data[getParent(m, x)].left = y;
			}
			else
			{
				m.data[getParent(m, x)].right = y;
			}
		}

		m.data[y].right = x;
		m.data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void destroy(Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);

		uint32_t x = VectorFn::getCount(m.data) - 1;

		if (x == m.root)
		{
			m.root = n;

			if (getLeft(m, x) != NIL)
			{
				m.data[getLeft(m, x)].parent = n;
			}
			if (getRight(m, x) != NIL)
			{
				m.data[getRight(m, x)].parent = n;
			}

			m.data[n] = m.data[x];
		}
		else
		{
			if (x != n)
			{
				if (x == getLeft(m, getParent(m, x)))
				{
					m.data[getParent(m, x)].left = n;
				}
				else if (x == getRight(m, getParent(m, x)))
				{
					m.data[getParent(m, x)].right = n;
				}

				if (getLeft(m, x) != NIL)
				{
					m.data[getLeft(m, x)].parent = n;
				}
				if (getRight(m, x) != NIL)
				{
					m.data[getRight(m, x)].parent = n;
				}

				m.data[n] = m.data[x];
			}
		}

		#ifdef RBTREE_VERIFY
			debugVerify(m, m.root);
		#endif // RBTREE_VERIFY

		VectorFn::popBack(m.data);
	}

	template <typename TKey, typename TValue>
	inline void insertFixup(Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.data), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.data), n);

		uint32_t x;
		uint32_t y;

		while (n != getRoot(m) && getColor(m, getParent(m, n)) == RED)
		{
			x = getParent(m, n);

			if (x == getLeft(m, getParent(m, x)))
			{
				y = getRight(m, getParent(m, x));

				if (getColor(m, y) == RED)
				{
					m.data[x].color = BLACK;
					m.data[y].color = BLACK;
					m.data[getParent(m, x)].color = RED;
					n = getParent(m, x);
					continue;
				}
				else
				{
					if (n == getRight(m, x))
					{
						n = x;
						rotateLeft(m, n);
						x = getParent(m, n);
					}

					m.data[x].color = BLACK;
					m.data[getParent(m, x)].color = RED;
					rotateRight(m, getParent(m, x));
				}
			}
			else
			{
				y = getLeft(m, getParent(m, x));

				if (getColor(m, y) == RED)
				{
					m.data[x].color = BLACK;
					m.data[y].color = BLACK;
					m.data[getParent(m, x)].color = RED;
					n = getParent(m, x);
					continue;
				}
				else
				{
					if (n == getLeft(m, x))
					{
						n = x;
						rotateRight(m, n);
						x = getParent(m, n);
					}

					m.data[x].color = BLACK;
					m.data[getParent(m, x)].color = RED;
					rotateLeft(m, getParent(m, x));
				}
			}
		}
	}

	template <typename TKey, typename TValue>
	inline uint32_t innerFind(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t x = m.root;

		while (x != m.sentinel)
		{
			if (m.data[x].pair.first < key)
			{
				if (getRight(m, x) == m.sentinel)
				{
					return x;
				}

				x = getRight(m, x);
			}
			else if (key < m.data[x].pair.first)
			{
				if (getLeft(m, x) == m.sentinel)
				{
					return x;
				}

				x = getLeft(m, x);
			}
			else
			{
				break;
			}
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t findOrFail(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t p = innerFind(m, key);

		if (p != m.sentinel && m.data[p].pair.first == key)
		{
			return p;
		}

		return NIL;
	}

	template <typename TKey, typename TValue>
	inline uint32_t findOrAdd(Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t p = innerFind(m, key);

		if (p != m.sentinel && m.data[p].pair.first == key)
		{
			return p;
		}

		typename Map<TKey, TValue>::Node n;
		n.key = key;
		n.value = TValue();
		n.color = RED;
		n.left = m.sentinel;
		n.right = m.sentinel;
		n.parent = NIL;

		if (p == m.sentinel)
		{
			m.root = n;
		}
		else
		{
			if (key < m.data[p].pair.first)
			{
				m.data[p].left = n;
			}
			else
			{
				m.data[p].right = n;
			}

			m.data[n].parent = p;
		}

		addFixup(m, n);
		m.data[m.root].color = BLACK;
		#ifdef RBTREE_VERIFY
			debugVerify(m, m.root);
		#endif // RBTREE_VERIFY
		return n;
	}
} // namespace MapInternalFn

namespace MapFn
{
	template <typename TKey, typename TValue>
	uint32_t getCount(const Map<TKey, TValue>& m)
	{
		RIO_ASSERT(VectorFn::getCount(m.data) > 0, "Bad Map"); // There should be at least sentinel
		return VectorFn::getCount(m.data) - 1;
	}

	template <typename TKey, typename TValue>
	inline bool has(const Map<TKey, TValue>& m, const TKey& key)
	{
		return MapInternalFn::findOrFail(m, key) != MapInternalFn::NIL;
	}

	template <typename TKey, typename TValue>
	inline const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault)
	{
		uint32_t p = MapInternalFn::innerFind(m, key);

		if (p != m.sentinel && m.data[p].pair.first == key)
		{
			return m.data[p].pair.second;
		}

		return deffault;
	}

	template <typename TKey, typename TValue>
	inline void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value)
	{
		typename Map<TKey, TValue>::Node node(*m.data.allocator);
		node.pair.first = key;
		node.pair.second = value;
		node.color = MapInternalFn::RED;
		node.left = m.sentinel;
		node.right = m.sentinel;
		node.parent = MapInternalFn::NIL;
		uint32_t n = VectorFn::pushBack(m.data, node);
		uint32_t x = m.root;
		uint32_t y = MapInternalFn::NIL;

		if (x == m.sentinel)
		{
			m.root = n;
		}
		else
		{
			while (x != m.sentinel)
			{
				y = x;

				if (key < m.data[x].pair.first)
				{
					x = m.data[x].left;
				}
				else
				{
					x = m.data[x].right;
				}
			}

			if (key < m.data[y].pair.first)
			{
				m.data[y].left = n;
			}
			else
			{
				m.data[y].right = n;
			}

			m.data[n].parent = y;
		}

		MapInternalFn::insertFixup(m, n);
		m.data[m.root].color = MapInternalFn::BLACK;
		#ifdef RBTREE_VERIFY
			MapInternalFn::debugVerify(m, m.root);
		#endif // RBTREE_VERIFY
	}

	template <typename TKey, typename TValue>
	inline void remove(Map<TKey, TValue>& m, const TKey& key)
	{
		using namespace MapInternalFn;

		uint32_t n = innerFind(m, key);

		if (!(m.data[n].pair.first == key))
		{
			return;
		}

		uint32_t x;
		uint32_t y;

		if (getLeft(m, n) == m.sentinel || getRight(m, n) == m.sentinel)
		{
			y = n;
		}
		else
		{
			y = getSuccessor(m, n);
		}

		if (getLeft(m, y) != m.sentinel)
		{
			x = getLeft(m, y);
		}
		else
		{
			x = getRight(m, y);
		}

		m.data[x].parent = getParent(m, y);

		if (getParent(m, y) != MapInternalFn::NIL)
		{
			if (y == getLeft(m, getParent(m, y)))
			{
				m.data[getParent(m, y)].left = x;
			}
			else
			{
				m.data[getParent(m, y)].right = x;
			}
		}
		else
		{
			m.root = x;
		}

		if (y != n)
		{
			m.data[n].pair.first = m.data[y].pair.first;
			m.data[n].pair.second = m.data[y].pair.second;
		}

		// Do the fixup
		if (getColor(m, y) == MapInternalFn::BLACK)
		{
			uint32_t y;

			while (x != m.root && getColor(m, x) == MapInternalFn::BLACK)
			{
				if (x == getLeft(m, getParent(m, x)))
				{
					y = getRight(m, getParent(m, x));

					if (getColor(m, y) == MapInternalFn::RED)
					{
						m.data[y].color = MapInternalFn::BLACK;
						m.data[getParent(m, x)].color = MapInternalFn::RED;
						rotateLeft(m, getParent(m, x));
						y = getRight(m, getParent(m, x));
					}

					if (getColor(m, getLeft(m, y)) == MapInternalFn::BLACK && getColor(m, getRight(m, y)) == MapInternalFn::BLACK)
					{
						m.data[y].color = MapInternalFn::RED;
						x = getParent(m, x);
					}
					else
					{
						if (getColor(m, getRight(m, y)) == MapInternalFn::BLACK)
						{
							m.data[getLeft(m, y)].color = MapInternalFn::BLACK;
							m.data[y].color = MapInternalFn::RED;
							rotateRight(m, y);
							y = getRight(m, getParent(m, x));
						}

						m.data[y].color = getColor(m, getParent(m, x));
						m.data[getParent(m, x)].color = MapInternalFn::BLACK;
						m.data[getRight(m, y)].color = MapInternalFn::BLACK;
						rotateLeft(m, getParent(m, x));
						x = m.root;
					}
				}
				else
				{
					y = getLeft(m, getParent(m, x));

					if (getColor(m, y) == MapInternalFn::RED)
					{
						m.data[y].color = MapInternalFn::BLACK;
						m.data[getParent(m, x)].color = MapInternalFn::RED;
						rotateRight(m, getParent(m, x));
						y = getLeft(m, getParent(m, x));
					}

					if (getColor(m, getRight(m, y)) == MapInternalFn::BLACK && getColor(m, getLeft(m, y)) == MapInternalFn::BLACK)
					{
						m.data[y].color = MapInternalFn::RED;
						x = getParent(m, x);
					}
					else
					{
						if (getColor(m, getLeft(m, y)) == MapInternalFn::BLACK)
						{
							m.data[getRight(m, y)].color = MapInternalFn::BLACK;
							m.data[y].color = MapInternalFn::RED;
							rotateLeft(m, y);
							y = getLeft(m, getParent(m, x));
						}

						m.data[y].color = getColor(m, getParent(m, x));
						m.data[getParent(m, x)].color = MapInternalFn::BLACK;
						m.data[getLeft(m, y)].color = MapInternalFn::BLACK;
						rotateRight(m, getParent(m, x));
						x = m.root;
					}
				}
			}

			m.data[x].color = MapInternalFn::BLACK;
		}

		destroy(m, y);
	 	#ifdef RBTREE_VERIFY
			MapInternalFn::debugVerify(m, m.root);
	 	#endif // RBTREE_VERIFY
	}

	template <typename TKey, typename TValue>
	void clear(Map<TKey, TValue>& m)
	{
		VectorFn::clear(m.data);

		m.root = 0;
		m.sentinel = 0;

		typename Map<TKey, TValue>::Node r(*m.data.allocator);
		r.left = MapInternalFn::NIL;
		r.right = MapInternalFn::NIL;
		r.parent = MapInternalFn::NIL;
		r.color = MapInternalFn::BLACK;
		VectorFn::pushBack(m.data, r);
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m)
	{
		return VectorFn::begin(m.data) + 1; // Skip sentinel at index 0
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m)
	{
		return VectorFn::end(m.data);
	}
} // namespace MapFn

template <typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Allocator& a)
	: data(a)
{
	MapFn::clear(*this);
}

template <typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::operator[](const TKey& key) const
{
	return MapFn::get(*this, key, TValue());
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka