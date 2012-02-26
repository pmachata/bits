/*
 * Implementation of singly linked list interface on top of
 * std::vector.
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

#ifndef _FORWARD_VEC_H_
#define _FORWARD_VEC_H_

#include <vector>

template <class T, class Allocator = std::allocator<T>>
class forward_vec
  : private std::vector<T, Allocator>
{
  typedef std::vector<T, Allocator> Super;
public:
  typedef typename Super::reverse_iterator iterator;
  typedef typename Super::const_reverse_iterator const_iterator;
  typedef typename Super::value_type value_type;
  typedef typename Super::size_type size_type;
  typedef typename Super::reference reference;
  typedef typename Super::pointer pointer;

  using Super::clear;
  using Super::empty;

  template<class InputIterator>
  forward_vec (InputIterator begin, InputIterator end)
  {
    Super tmp (begin, end);
    Super::insert (Super::end (), tmp.rbegin (), tmp.rend ());
  }

  forward_vec ()
  {}

  void
  swap (forward_vec &other)
  {
    std::swap (*this, other);
  }

  void
  push_front (T const &emt)
  {
    Super::push_back (emt);
  }

  void
  pop_front ()
  {
    Super::pop_back ();
  }

  typename Super::reference
  front ()
  {
    return Super::back ();
  }

  iterator
  begin ()
  {
    return Super::rbegin ();
  }

  const_iterator
  begin () const
  {
    return Super::rbegin ();
  }

  const_iterator
  cbegin () const
  {
    return Super::rbegin ();
  }

  iterator
  end ()
  {
    return Super::rend ();
  }

  const_iterator
  end () const
  {
    return Super::rend ();
  }

  const_iterator
  cend () const
  {
    return Super::rend ();
  }

  bool
  operator == (forward_vec const &other) const
  {
    return *(Super const *)this == other;
  }

private:
  typename Super::iterator
  underlying_iter (const_iterator it, size_type &i)
  {
    i = it - begin ();
    size_type ri = this->size () - 1 - i;
    return Super::begin () + ri;
  }

public:
  iterator
  erase_after (const_iterator it)
  {
    size_type i;
    auto jt = underlying_iter (it, i) - 1;
    if (it + 1 < end ())
      Super::erase (jt);
    return begin () + i;
  }

  iterator
  erase_after (iterator it)
  {
    return erase_after (this->cbegin () + (it - this->begin ()));
  }

  iterator
  erase_after (const_iterator first, const_iterator last)
  {
    size_type a = first - this->cbegin ();
    for (size_type count = last - first; count > 0; --count)
      erase_after (this->cbegin () + a);
    return this->begin () + a;
  }

  void
  insert_after (const_iterator it, const T &value)
  {
    size_type i;
    auto jt = underlying_iter (it, i);
    Super::insert (jt, value);
  }

  void
  resize (size_type n, T const &value = T ())
  {
    // taken from slist.hh
    if (n == 0)
      {
	clear ();
	return;
      }

    iterator it = begin ();
    iterator jt = it;
    size_type i = 0;
    for (; i < n - 1; ++i)
      if (it == end ())
	break;
      else
	jt = it++;
    if (it != end ())
      erase_after (it, end ());
    else
      for (; i < n; ++i)
	insert_after (jt, value);
  }
};

#endif /* _FORWARD_VEC_H_ */
