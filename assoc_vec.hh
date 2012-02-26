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
  : private std::vector<std::pair<Key, T>, Allocator>
{
  typedef std::vector<std::pair<Key, T>, Allocator> Super;
  bool _sorted;

  bool
  subset (assoc_vec const &other) const
  {
    for (auto it = begin (); it != end (); ++it)
      {
	auto jt = other.find (it->first);
	if (jt == other.end ()
	    || jt->second != it->second)
	  return false;
      }
    return true;
  }

public:
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<Key, T> value_type;
  typedef value_type &reference;
  typedef value_type const &const_reference;
  typedef value_type *pointer;
  typedef value_type const *const_pointer;

  using typename Super::iterator;
  using typename Super::const_iterator;
  using typename Super::size_type;
  using Super::size;
  using Super::clear;

  bool
  operator == (assoc_vec const &other) const
  {
    return subset (other) && other.subset (*this);
  }

  bool
  operator != (assoc_vec const &other) const
  {
    return !(other == *this);
  }

  typename Super::iterator
  begin ()
  {
    return Super::begin ();
  }

  typename Super::iterator
  end ()
  {
    return Super::end ();
  }

  typename Super::const_iterator
  begin () const
  {
    return Super::cbegin ();
  }

  typename Super::const_iterator
  end () const
  {
    return Super::cend ();
  }

  typename Super::const_iterator
  cbegin () const
  {
    return Super::cbegin ();
  }

  typename Super::const_iterator
  cend () const
  {
    return Super::cend ();
  }

private:
  template<class Iterator>
  Iterator
  find (Iterator b, Iterator e, key_type const &key) const
  {
    return std::find_if (b, e, [key] (value_type const &a) {
	return a.first == key;
      });
  }

public:
  typename Super::iterator
  find (key_type const &key)
  {
    return find (begin (), end (), key);
  }

  typename Super::const_iterator
  find (key_type const &key) const
  {
    return find (cbegin (), cend (), key);
  }

  std::pair<typename Super::iterator, bool>
  insert (const_reference emt)
  {
    auto it = find (emt.first);

    if (it == end ())
      {
	Super::push_back (emt);
	return std::make_pair (begin () + (size () - 1), true);
      }
    else
      {
	*it = emt;
	return std::make_pair (it, false);
      }
  }

  std::pair<typename Super::iterator, bool>
  insert (typename Super::const_iterator, const_reference emt)
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
