/*
 * Implementation of singly linked list allocated on stack given
 * maximum number of elements.
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

#ifndef _SLIST_H_
#define _SLIST_H_

#include <cassert>
#include <cstddef>
#include <memory>
#include <cinttypes>

template<class T, size_t N>
class slist
{
public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;

protected:
  typedef typename std::conditional<(N < 256), uint8_t,
    typename std::conditional<(N < 65536), uint16_t, size_t>::type >::type
  index_type;

  struct slot
  {
    unsigned char bytes[sizeof (T)]; // payload
  };

  slot _slots[N];
  index_type _nexts[N];
  index_type _head;
  index_type _free;

  template<class This, class Slist>
  class iterator_builder
    : public std::iterator<std::forward_iterator_tag, T>
  {
  protected:
    Slist _parent;
    index_type _pos;

    iterator_builder (Slist parent, index_type pos)
      : _parent (parent)
      , _pos (pos)
    {}

    iterator_builder (iterator_builder const &copy)
      : _parent (copy._parent)
      , _pos (copy._pos)
    {}

  public:
    bool
    operator == (This const &other) const
    {
      assert (_parent == other._parent);
      return _pos == other._pos;
    }

    bool
    operator != (This const &other) const
    {
      return !(*this == other);
    }

    This &
    operator ++ ()
    {
      assert (_pos != N);
      _pos = _parent->_nexts[_pos];
      return *(This *)this;
    }

    This
    operator ++ (int)
    {
      This copy = *(This *)this;
      ++*this;
      return copy;
    }
  };

public:

  class iterator
    : public iterator_builder<iterator, slist *>
  {
    typedef iterator_builder<iterator, slist *> Super;
    friend class slist;

    iterator (slist *parent, index_type pos)
      : Super (parent, pos)
    {}

  public:
    iterator ()
      : Super (NULL, N)
    {}

    iterator (iterator const &copy)
      : Super (copy)
    {}

    reference
    operator * ()
    {
      unsigned char *bytes = this->_parent->_slots[this->_pos].bytes;
      return *reinterpret_cast<pointer> (bytes);
    }

    pointer
    operator -> ()
    {
      return &**this;
    }
  };

  class const_iterator
    : public iterator_builder<const_iterator, slist const *>
  {
    typedef iterator_builder<const_iterator, slist const *> Super;
    friend class slist;

    const_iterator (slist const *parent, index_type pos)
      : Super (parent, pos)
    {}

  public:
    const_iterator ()
      : Super (NULL, N)
    {}

    const_iterator (const_iterator const &copy)
      : Super (copy)
    {}

    const_iterator (iterator const &other)
      : Super (other._parent, other._pos)
    {}

    const_reference
    operator * ()
    {
      unsigned char const *bytes = this->_parent->_slots[this->_pos].bytes;
      return *reinterpret_cast<const_pointer> (bytes);
    }

    const_pointer
    operator -> ()
    {
      return &**this;
    }
  };

private:

  void
  init ()
  {
    _head = N;
    _free = 0;
    for (size_t i = 0; i < N; ++i)
      _nexts[i] = i + 1;
  }

  void
  push_back (const T &value, index_type &tail)
  {
    index_type i = take_slot (value);
    if (tail != N)
      _nexts[tail] = i;
    tail = i;
  }

public:

  slist ()
  {
    init ();
  }

  slist (slist const &copy)
    : _head (N)
  {
    *this = slist (copy.begin (), copy.end ());
  }

  template <class InputIterator>
  slist (InputIterator first, InputIterator last)
  {
    init ();
    index_type tail = N;
    while (first != last)
      push_back (*first++, tail);
    if (tail != N)
      {
	_head = 0;
	_nexts[tail] = N;
      }
  }

  slist (size_t n, T const &value = T ())
  {
    init ();
    for (size_t i = 0; i < n; ++i)
      push_front (value);
  }

  ~slist ()
  {
    for (auto it = begin (); it != end (); ++it)
      it->~T ();
  }

  void
  swap (slist &other)
  {
    std::swap (_slots, other._slots);
    std::swap (_nexts, other._nexts);
    std::swap (_head, other._head);
    std::swap (_free, other._free);
  }

  slist &
  operator = (slist other)
  {
    swap (other);
    return *this;
  }

  void
  clear ()
  {
    slist empty;
    swap (empty);
  }

private:
  void
  check_space () const
  {
    if (_free == N)
      throw std::bad_alloc ();
  }

  index_type
  take_slot (const T &value)
  {
    check_space ();
    index_type i = _free;
    slot &slt = _slots[i];
    _free = _nexts[i];
    new (slt.bytes) T (value);
    return i;
  }

  void
  return_slot (index_type i)
  {
    unsigned char *bytes = _slots[i].bytes;
    T &object = *reinterpret_cast<pointer> (bytes);
    object.~T ();
    _nexts[i] = _free;
    _free = i;
  }

public:
  void
  push_front (const T &value)
  {
    index_type i = take_slot (value);
    _nexts[i] = _head;
    _head = i;
  }

  void
  pop_front ()
  {
    index_type i = _head;
    _head = _nexts[i];
    return_slot (i);
  }

  iterator
  erase_after (const_iterator it)
  {
    index_type next = _nexts[it._pos];
    index_type nnext = _nexts[next];
    _nexts[it._pos] = nnext;
    return_slot (next);
    return iterator (this, nnext);
  }

  iterator
  erase_after (const_iterator first, const_iterator last)
  {
    while (_nexts[first._pos] != last._pos)
      erase_after (first);
    return iterator (this, last._pos);
  }

  reference
  front ()
  {
    return *begin ();
  }

  const_reference
  front () const
  {
    return *begin ();
  }

  bool
  empty () const
  {
    return _head == N;
  }

  iterator
  begin ()
  {
    return iterator (this, _head);
  }

  const_iterator
  begin () const
  {
    return const_iterator (this, _head);
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

  bool
  operator == (slist const &other) const
  {
    const_iterator it = begin ();
    const_iterator jt = other.begin ();
    for (; it != end () && jt != other.end (); ++it, ++jt)
      if (*it != *jt)
	return false;
    return (it == end ()) == (jt == other.end ());
  }

  bool
  operator != (slist const &other) const
  {
    return !(*this == other);
  }
};

namespace std
{
  template <class T, size_t N>
  struct tuple_size<slist<T, N>>
  {
    enum { value = N };
  };
}

#endif /* _SLIST_H_ */
