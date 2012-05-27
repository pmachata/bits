/*
 * Implementation of prime number iterator based on Sieve of
 * Eratosthenes.
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

#ifndef _PRIME_ITERATOR_H_
#define _PRIME_ITERATOR_H_

#include <vector>
#include <cmath>
#include <iostream>

template <class T = unsigned>
class prime_iterator
  : public std::iterator<std::input_iterator_tag, T>
{
  typedef std::vector<bool> sieve_t;
  sieve_t sieve;
  T cur;

  void
  extend_sieve (sieve_t::size_type end)
  {
    if (sieve.size () >= end)
      return;

    sieve_t::size_type base = sieve.size ();
    sieve.resize (end, true);

    sieve_t::size_type last = (sieve_t::size_type)std::sqrt (end);
    if (last * last < end)
      ++last;

    /* This sub-sieve is necessary so that we know which factors were
       processed in this round.  What's in SIEVE just tells us what
       was processed in the past, but for the new part of sieve, we
       need to do the computation again.  */
    std::vector<bool> local (last, true);

    for (T i = 2; i < last; ++i)
      if (local[i])
	{
	  for (T j = i; j < last; j += i)
	    local[j] = false;
	  for (T j = ((base + i - 1)/ i) * i; j < end; j += i)
	    sieve[j] = false;
	}
  }

  bool
  is_prime (T val)
  {
    extend_sieve (val + 1);
    return sieve[val];
  }

public:
  prime_iterator (T initial = 2)
    : cur (initial)
  {
    extend_sieve (2 * initial);
    if (!is_prime (initial))
      ++*this;
  }

  prime_iterator &
  operator = (T rhs)
  {
    cur = rhs;
    extend_sieve (2 * cur);
    if (!is_prime (cur))
      ++*this;
    return *this;
  }

  T
  operator * () const
  {
    return cur;
  }

  prime_iterator &
  operator ++ ()
  {
    do
      if (cur >= sieve.size () - 1)
	extend_sieve (sieve.size () * 2);
    while (!sieve[++cur]);
    return *this;
  }

  prime_iterator
  operator ++ (int)
  {
    prime_iterator copy = *this;
    ++*this;
    return copy;
  }

  bool
  operator < (T rhs) const
  {
    return cur < rhs;
  }

  bool
  operator == (T rhs) const
  {
    return cur == rhs;
  }

  bool
  operator == (prime_iterator rhs) const
  {
    return *this == rhs.cur;
  }

  bool
  operator < (prime_iterator rhs) const
  {
    return *this < rhs.cur;
  }

  template <class U>
  bool
  operator != (U rhs) const
  {
    return !(*this == rhs);
  }

  template <class U>
  bool
  operator <= (U rhs) const
  {
    return *this == rhs || *this < rhs;
  }

  template <class U>
  bool
  operator > (U rhs) const
  {
    return !(*this <= rhs);
  }

  template <class U>
  bool
  operator >= (U rhs) const
  {
    return !(*this < rhs);
  }
};

#endif /* _PRIME_ITERATOR_H_ */
