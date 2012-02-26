/*
 * Implementation of associative container on top of std::vector.
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

#ifndef _ASSOC_VEC_H_
#define _ASSOC_VEC_H_

#include <vector>
#include <functional>
#include <algorithm>

template<class Key, class T,
	 class Compare = std::less<Key>,
	 class Allocator = std::allocator<std::pair<Key, T> > >
class assoc_vec
{
  typedef std::vector<std::pair<Key, T>, Allocator> Vec;
  mutable Vec _vec;

public:
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<Key, T> value_type;
  typedef value_type &reference;
  typedef value_type const &const_reference;
  typedef value_type *pointer;
  typedef value_type const *const_pointer;

  typedef typename Vec::iterator iterator;
  typedef typename Vec::const_iterator const_iterator;
  typedef typename Vec::size_type size_type;

  size_type
  size () const
  {
    return _vec.size ();
  }

  void
  clear ()
  {
    _vec.clear ();
  }

  assoc_vec ()
  {}

  assoc_vec (assoc_vec const &other)
    : _vec (other._vec)
  {}

  template <class Iterator>
  assoc_vec (Iterator begin, Iterator end)
    : _vec (begin, end)
  {}

  bool
  operator == (assoc_vec const &other) const
  {
    return _vec == other._vec;
  }

  bool
  operator != (assoc_vec const &other) const
  {
    return !(other == *this);
  }

  iterator
  begin ()
  {
    return _vec.begin ();
  }

  iterator
  end ()
  {
    return _vec.end ();
  }

  const_iterator
  begin () const
  {
    return _vec.cbegin ();
  }

  const_iterator
  end () const
  {
    return _vec.cend ();
  }

  const_iterator
  cbegin () const
  {
    return _vec.cbegin ();
  }

  const_iterator
  cend () const
  {
    return _vec.cend ();
  }

private:
  template<class Iterator>
  Iterator
  lbound (Iterator b, Iterator e, key_type const &key) const
  {
    return std::lower_bound (b, e, key,
			     [] (value_type const &a, key_type const &key)
			     {
			       return a.first < key;
			     });
  }

  template<class Iterator>
  Iterator
  found (Iterator it, Iterator end, key_type const &key) const
  {
    if (it != end && it->first == key)
      return it;
    else
      return end;
  }

public:
  iterator
  find (key_type const &key)
  {
    return found (lbound (begin (), end (), key), end (), key);
  }

  const_iterator
  find (key_type const &key) const
  {
    return found (lbound (cbegin (), cend (), key), cend (), key);
  }

  std::pair<iterator, bool>
  insert (const_reference emt)
  {
    auto it = lbound (begin (), end (), emt.first);

    if (found (it, end (), emt.first) == end ())
      {
	size_type i = it - begin ();
	_vec.insert (it, emt);
	return std::make_pair (begin () + i, true);
      }
    else
      return std::make_pair (it, false);
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
};

#endif /* _ASSOC_VEC_H_ */
