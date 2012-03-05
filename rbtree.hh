/*
 * Implementation of red-black tree on top of std::vector.
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

#include <vector>
#include <functional>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <iostream>

template <class Key, class T, class Compare = std::less<Key> >
class rbtree
{
public:
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<Key, T> value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef Compare key_compare;
  typedef value_type &reference;
  typedef value_type const &const_reference;
  typedef value_type *pointer;
  typedef value_type const *const_pointer;

  class iterator;
  class const_iterator;

private:
  struct slot
  {
    unsigned char bytes[sizeof (value_type)]; // payload
  };

  std::vector<slot> _slots;
  std::vector<bool> _taken;
  Compare _cmp;
  size_type _size;

  bool
  is_subset (rbtree const &other) const
  {
    for (auto it = begin (); it != end (); ++it)
      {
	auto jt = other.find (it->first);
	if (jt == other.end ()
	    || _cmp (it->second, jt->second)
	    || _cmp (jt->second, it->second))
	  return false;
      }
    return true;
  }

  value_type const &
  slot (size_type pos) const
  {
    unsigned char const *bytes = _slots[pos].bytes;
    return *reinterpret_cast<value_type const *> (bytes);
  }

  void
  reserve (size_type size)
  {
    if (_slots.size () < size)
      {
	_slots.resize (size);
	_taken.resize (size, false);
      }
  }

  value_type &
  slot (size_type pos)
  {
    unsigned char *bytes = _slots[pos].bytes;
    return *reinterpret_cast<value_type *> (bytes);
  }

  size_type
  left_child (size_type pos) const
  {
    return 2 * pos + 1;
  }

  size_type
  right_child (size_type pos) const
  {
    return 2 * pos + 2;
  }

  bool
  valid (size_type pos) const
  {
    return pos < _slots.size () && _taken[pos];
  }

  template<class This, class RbTreePtr, class ValType>
  class iterator_builder
    : public std::iterator<std::forward_iterator_tag, value_type>
  {
    friend class rbtree;

    RbTreePtr _parent;
    size_type _pos;
    std::vector<size_type> _next;

    static This
    end (RbTreePtr parent)
    {
      return This (parent, parent->_slots.size ());
    }

    void
    push_if_valid (size_type pos)
    {
      assert (_parent != NULL);
      if (_parent->valid (pos))
	_next.push_back (pos);
    }

    void
    visit ()
    {
      push_if_valid (_parent->right_child (_pos));
      push_if_valid (_parent->left_child (_pos));
    }

    iterator_builder (RbTreePtr ptr, size_t pos,
		      std::vector<size_type> next = std::vector<size_type> ())
      : _parent (ptr)
      , _pos (pos)
      , _next (next)
    {
      if (_parent != NULL)
	visit ();
    }

    This *
    self ()
    {
      return reinterpret_cast<This *>(this);
    }

    void
    find (Key const &key)
    {
      value_type const &s = **this;
      if (this->_parent->_cmp (key, s.first))
	{
	  // Left branch: leave the right next in the queue.
	  if (!_next.empty ())
	    _next.pop_back ();
	  _pos = this->_parent->left_child (this->_pos);
	}
      else
	{
	  // Right branch: drop both next's in queue.
	  if (!_next.empty ())
	    _next.pop_back ();
	  if (!_next.empty ())
	    _next.pop_back ();
	  // Make sure they are not accidentally equal.
	  assert (this->_parent->_cmp (s.first, key));
	  _pos = this->_parent->right_child (this->_pos);
	}
      visit ();
    }

  public:
    bool
    operator == (This const &other) const
    {
      return _parent == other._parent
	&& (_pos == other._pos
	    || (!_parent->valid (_pos) && !_parent->valid (other._pos)));
    }

    bool
    operator != (This const &other) const
    {
      return !(*this == other);
    }

    This &
    operator ++ ()
    {
      if (!_parent->valid (_pos))
	throw std::runtime_error ("called ++ at end iterator");

      if (_next.empty ())
	*this = end (_parent);
      else
	{
	  _pos = _next.back ();
	  _next.pop_back ();
	  visit ();
	}

      return *self ();
    }

    This
    operator ++ (int)
    {
      This copy = *this;
      ++*this;
      return copy;
    }

    ValType *
    operator -> () const
    {
      return &this->_parent->slot (this->_pos);
    }

    ValType &
    operator * () const
    {
      return *this->operator -> ();
    }
  };

public:

  class iterator
    : public iterator_builder<iterator, rbtree *, value_type>
  {
    friend class rbtree;
    typedef iterator_builder<iterator, rbtree *, value_type> Super;

    iterator (rbtree *t, size_type pos)
      : Super (t, pos)
    {}

    iterator (const_iterator const &other) /* private */
      : Super ((rbtree *)other._parent, other._pos, other._next)
    {}

  public:
    iterator ()
      : Super (NULL, 0)
    {}
  };

  class const_iterator
    : public iterator_builder<const_iterator, rbtree const *,
			      value_type const>
  {
    friend class rbtree;
    typedef iterator_builder<const_iterator, rbtree const *,
			     value_type const> Super;

    const_iterator (rbtree const *t, size_type pos)
      : Super (t, pos)
    {}

  public:
    const_iterator (iterator const &other)
      : Super (other._parent, other._pos, other._next)
    {}

    const_iterator ()
      : Super (NULL, 0)
    {}
  };

  explicit rbtree (Compare const &cmp = Compare ())
    : _cmp (cmp)
    , _size (0)
  {}

  rbtree (rbtree const &copy)
  {
    *this = rbtree (copy.begin (), copy.end (), copy._cmp);
  }

  template <class Iterator>
  rbtree (Iterator first, Iterator last,
	  Compare const &cmp = Compare ())
    : _cmp (cmp)
    , _size (0)
  {
    for (Iterator it = first; it != last; ++it)
      insert (*it);
  }

  rbtree &
  operator = (rbtree other)
  {
    swap (other);
    return *this;
  }

  void
  swap (rbtree &other)
  {
    ck ();
    _slots.swap (other._slots);
    _taken.swap (other._taken);
    std::swap (_cmp, other._cmp);
    std::swap (_size, other._size);
    ck ();
  }

  bool
  operator == (rbtree const &other) const
  {
    ck ();
    dump ();
    return is_subset (other) && other.is_subset (*this);
  }

  bool
  operator != (rbtree const &other) const
  {
    ck ();
    return !(*this == other);
  }

  void
  clear ()
  {
    ck ();
    *this = rbtree ();
  }

  void
  ck () const
  {
    assert (_slots.size () == _taken.size ());
  }

  void
  dump () const
  {
    return;
    std::cout << _slots.size () << '/' << _taken.size ();
    for (size_t i = 0; i < _slots.size (); ++i)
      if (_taken[i])
	std::cout << "\t" << i << ":" << slot (i).first
		  << "=" << slot (i).second;
    std::cout << std::endl;
    for (auto it = begin (); it != end (); ++it)
      std::cout << "\t" << it->first << "=" << it->second << std::flush;
    std::cout << std::endl;
  }

  std::pair<iterator, bool>
  insert (value_type const &emt)
  {
    ck ();
    iterator it = find (emt.first);
    if (it != end ())
      return std::make_pair (it, false);

    reserve (it._pos + 1);
    new (_slots[it._pos].bytes) value_type (emt);
    _taken[it._pos] = true;
    ++_size;
    dump ();
    ck ();
    return std::make_pair (begin (), true);
  }

  iterator
  insert (const_iterator, const_reference emt)
  {
    return insert (emt).first;
  }

  template <class InputIterator>
  void
  insert (InputIterator first, InputIterator last)
  {
    ck ();
    for (; first != last; ++first)
      insert (*first);
    ck ();
  }

  const_iterator
  find (Key const& key) const
  {
    ck ();
    const_iterator it;
    for (it = begin (); it != end (); it.find (key))
      if (!_cmp (it->first, key)
	  && !_cmp (key, it->first))
	return it;
    return it;
  }

  iterator
  find (Key const& key)
  {
    ck ();
    return iterator (const_cast<rbtree const *> (this)->find (key));
  }

  iterator
  begin ()
  {
    ck ();
    return iterator (this, 0);
  }

  const_iterator
  begin () const
  {
    ck ();
    return const_iterator (this, 0);
  }

  const_iterator
  cbegin () const
  {
    ck ();
    return begin ();
  }

  iterator
  end ()
  {
    ck ();
    return iterator::end (this);
  }

  const_iterator
  end () const
  {
    ck ();
    return const_iterator::end (this);
  }

  const_iterator
  cend () const
  {
    ck ();
    return end ();
  }

  size_type
  size () const
  {
    ck ();
    return _size;
  }
};
