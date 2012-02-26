/*
 * Implementation of fixed-size double-hashing hash table.  Inspired
 * by the book Algorithms in C by Robert Sedgewick.
 *
 * Copyright (C) 2012 Petr Machata
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Caveat: we use std::array to store the hash table proper.  For that
 * reason we can't make the key part of the bucket const, and it's
 * therefore possible to change the key value through an iterator.
 * (Not const_iterator though.)
 *
 * Removal was not implemented.  */

#include <cstddef>
#include <array>
#include <bitset>
#include <utility>
#include <functional>
#include <type_traits>

// N.B. This might not be suitable for huge tables.
struct default_secondary_hash
{
  typedef size_t result_type;
  typedef size_t argument_type;
  size_t
  operator () (size_t v) const
  {
    return v % 97 + 1;
  }
};

// We use this as secondary hash for all small tables (the exact value
// of small is defined in the hashtab template).  The reason is that
// for small tables, the default secondary hash would easily wrap
// around and get back to the original position, and the insertion
// would end in a loop.  Iterative enumeration of all available
// positions is well affordable for small tables.
struct trivial_secondary_hash
{
  typedef size_t result_type;
  typedef size_t argument_type;
  size_t
  operator () (size_t v) const
  {
    return 1;
  }
};

template <class Key, class T, size_t N,
	  class Hash1 = std::hash<Key>,
	  class Hash2 = typename std::conditional<(N > 100),
						  default_secondary_hash,
						  trivial_secondary_hash>::type,
	  class Equal = std::equal_to<Key>>
struct hashtab
{
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<key_type, mapped_type> value_type;
  typedef size_t size_type;
  typedef Hash1 primary_hasher;
  typedef Hash2 secondary_hasher;
  typedef Equal key_equal;
  typedef value_type &reference;
  typedef value_type const &const_reference;
  typedef value_type *pointer;
  typedef value_type const *const_pointer;

  class iterator;
  class const_iterator;

private:

  std::array<value_type, N> _table;
  std::bitset<N> _taken;
  size_type _size;

  primary_hasher _hash1;
  secondary_hasher _hash2;
  key_equal _eq;

  const_iterator
  find_slot (key_type const &e, bool &found) const
  {
    size_type pos = _hash1 (e) % N;
    size_type d = _hash2 (pos);
    found = false;
    for (; _taken[pos]; pos = (pos + d) % N)
      if (_eq (_table[pos].first, e))
	{
	  found = true;
	  break;
	}
    return const_iterator (this, pos);
  }

  template<class This, class Hashtab>
  class iterator_builder
    : public std::iterator<std::forward_iterator_tag, value_type>
  {
  protected:
    Hashtab _parent;
    size_type _pos;

    iterator_builder (Hashtab parent, size_type pos)
      : _parent (parent)
      , _pos (pos)
    {}

    iterator_builder (Hashtab parent)
      : _parent (parent)
      , _pos (0)
    {
      find_next ();
    }

    inline bool
    valid () const
    {
      return _pos < N;
    }

    inline void
    find_next ()
    {
	for (; valid () && !_parent->_taken[_pos]; ++_pos)
	  ;
    }

    inline void
    shift ()
    {
      if (valid ())
	{
	  ++_pos;
	  find_next ();
	}
    }

  public:

    void
    swap (iterator_builder &other)
    {
      std::swap (_pos, other._pos);
      std::swap (_parent, other._parent);
    }

    // Comparison operators are only defined over iterators that come
    // from the same container.  We might check for this if some sort
    // of DEBUG symbol is defined.
    bool
    operator < (This const &other) const
    {
      return this->pos < other._pos;
    }

    bool
    operator == (This const &other) const
    {
      return this->_pos == other._pos;
    }

    bool
    operator != (This const &other) const
    {
      return !(*this == other);
    }

    This &
    operator = (This other)
    {
      This::swap (other);
      return *this;
    }

    This &
    operator ++ ()
    {
      this->shift ();
      return *(This *)this;
    }

    This
    operator ++ (int)
    {
      This copy = *this;
      ++*this;
      return copy;
    }
  };

public:

  class iterator
    : public iterator_builder<iterator, hashtab *>
  {
    typedef iterator_builder<iterator, hashtab *> Super;
    friend class hashtab;

    explicit iterator (hashtab *parent)
      : Super (parent)
    {}

    iterator (hashtab *parent, size_type pos)
      : Super (parent, pos)
    {}

  public:
    iterator ()
      : Super (NULL, 0)
    {}

    value_type &
    operator * ()
    {
      return this->_parent->_table[this->_pos];
    }

    value_type *
    operator -> ()
    {
      return &**this;
    }
  };

  class const_iterator
    : public iterator_builder<const_iterator, hashtab const *>
  {
    typedef iterator_builder<const_iterator, hashtab const *> Super;
    friend class hashtab;

    explicit const_iterator (hashtab const *parent)
      : Super (parent)
    {}

    const_iterator (hashtab const *parent, size_type pos)
      : Super (parent, pos)
    {}

  public:
    const_iterator ()
      : Super (NULL, 0)
    {}

    const_iterator (iterator const &other)
      : Super (other._parent, other._pos)
    {}

    value_type const &
    operator * ()
    {
      return this->_parent->_table[this->_pos];
    }

    value_type const *
    operator -> ()
    {
      return &**this;
    }
  };

  hashtab ()
    : _size (0)
  {}

  void
  swap (hashtab &other)
  {
    using std::swap;
    swap (_table, other._table);
    swap (_taken, other._taken);
    swap (_size, other._size);
    swap (_hash1, other._hash1);
    swap (_hash2, other._hash2);
    swap (_eq, other._eq);
  }

  hashtab &
  operator = (hashtab other)
  {
    swap (other);
    return *this;
  }

private:
  // We have to ignore what's in the unused (and uninitialized)
  // portions of the table.
  bool
  equal_slots (hashtab const &other) const
  {
    // N.B. at this point we already know that _taken == other._taken
    for (size_type i = 0; i < N; ++i)
      if (_taken[i] && _table[i] != other._table[i])
	return false;
    return true;
  }

  bool
  contains_all (hashtab const &other, bool just_keys) const
  {
    for (const_iterator it = other.begin (); it != other.end (); ++it)
      {
	const_iterator jt = find (it->first);
	if (jt == end () || (!just_keys && jt->second != it->second))
	  return false;
      }
    return true;
  }

public:
  bool
  operator == (hashtab const &other) const
  {
    if (_size != other._size)
      return false;

    // This may fail even if the tables are equal, but it might work,
    // and if it does, we can avoid the following (expensive) check.
    if (_taken == other._taken && equal_slots (other))
      return true;

    // The original idea was to use the same algorithm that
    // unordered_map uses, viz iterating "buckets" with local
    // iterators and testing whether is_permutation holds between this
    // chain and the one that's in OTHER.  But the fact that chain
    // that starts at position 1 contains positions [1, 3, 5, 7]
    // doesn't imply that the chain that starts at 3 contains
    // positions [3, 5, 7].  The increment is determined by the
    // secondary hash (due Sedgewick).  So in effect, we have N
    // independent chains, and checking all those is clearly a waste
    // of cycles.
    //
    // So instead we just manually go through keys and check their
    // presence.  Since find is amortized constant time operation, the
    // cost of this comparison is 2*N.  The advantage is that this
    // will keep working when (if) deletion is implemented.
    //
    // First check whether all THIS keys are in OTHER, next check the
    // other way around and also check that values are equal.
    return (contains_all (other, true)
	    && other.contains_all (*this, false));
  }

  bool
  operator != (hashtab const &other) const
  {
    return !(*this == other);
  }

  std::pair<iterator, bool>
  insert (const_reference emt)
  {
    if (_size == N)
      // Or should we raise exception?
      return std::make_pair (end (), false);

    bool found;
    const_iterator it = find_slot (emt.first, found);
    // N.B. """Inserts t if and only if there is no element in the
    // container with key equivalent to the key of t."""
    if (found)
      return std::make_pair (iterator (this, it._pos), false);

    _taken[it._pos] = true;
    _table[it._pos] = emt;
    ++_size;

    return std::make_pair (iterator (this, it._pos), true);
  }

  std::pair<iterator, bool>
  insert (const_iterator, const_reference emt)
  {
    return insert (emt);
  }

  template <class InputIterator>
  void
  insert (InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      insert (*first);
  }

  const_iterator
  find (key_type const &e) const
  {
    bool found;
    const_iterator it = find_slot (e, found);
    if (found)
      return it;
    else
      return end ();
  }

  iterator
  find (key_type const &e)
  {
    const_iterator it = const_cast<hashtab const *> (this)->find (e);
    return iterator (this, it._pos);
  }

  size_type
  size () const
  {
    return _size;
  }

  void
  clear ()
  {
    *this = hashtab ();
  }

  iterator
  begin ()
  {
    return iterator (this);
  }

  const_iterator
  begin () const
  {
    return const_iterator (this);
  }

  const_iterator
  cbegin () const
  {
    return begin ();
  }

  iterator
  end ()
  {
    return iterator (this, N);
  }

  const_iterator
  end () const
  {
    return const_iterator (this, N);
  }

  const_iterator
  cend () const
  {
    return end ();
  }
};

namespace std
{
  template <class Key, class T, size_t N, class Hash1, class Hash2>
  struct tuple_size<hashtab<Key, T, N, Hash1, Hash2>>
  {
    enum { value = N };
  };
}

template <class Key, class T, size_t N, class Hash1, class Hash2>
void
swap (hashtab<Key, T, N, Hash1, Hash2> &ht1,
      hashtab<Key, T, N, Hash1, Hash2> &ht2)
{
  ht1.swap (ht2);
}
