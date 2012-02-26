/*
 * Test suite for fixed-size double-hashing hash table.
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

#include "hash.hh"
#include "tests.hh"

#include <vector>
#include <algorithm>
#include <map>
#include <cassert>
#include <iostream>

template <class Key>
struct silly_hash
{
  typedef size_t result_type;
  typedef Key const &argument_type;
  size_t
  operator () (Key const &i) const
  {
    return 0;
  }
};

struct hash_int2
{
  typedef size_t result_type;
  typedef int argument_type;
  size_t
  operator () (int i) const
  {
    return i * 2654435761;
  }
};

struct hash_int3
{
  typedef size_t result_type;
  typedef int argument_type;
  size_t
  operator () (int i) const
  {
    int a = i;
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
  }
};

struct hash_string
{
  typedef size_t result_type;
  typedef std::string const &argument_type;
  size_t
  operator () (std::string const &str) const
  {
    unsigned long h = 5381;
    for (auto it = str.begin (); it != str.end (); ++it)
      h = h * 33 ^ *it;
    return h;
  }
};

template <class T, class U>
void
dummy_insert_test (T const &test, U &h)
{
  std::cout << "d" << std::flush;
  for (auto i = test.begin (); i != test.end (); ++i)
    {
      // insert a different element
      {
	auto p = h.insert (std::make_pair (*i, *i + *i));
	assert (p.first != h.end ());
	assert (!p.second);
	assert (p.first->second == *i);  // Nothing has changed
      }
    }
}

template <class T, class U>
void
membership_tests (T const &test, U &h)
{
  std::cout << "." << std::flush;
  for (auto i = test.begin (); i != test.end (); ++i)
    {
      auto it = h.find (*i);
      assert (it != h.end ());
      assert (it->first == *i);
    }

  std::cout << "." << std::flush;
  for (auto it = h.begin (); it != h.end (); ++it)
    {
      assert (std::find (test.begin (), test.end (), it->first) != test.end ());
      assert (it->first == (*it).second);
      auto jt = it;
      assert (it == jt);
    }
}

template <class H, int M = std::tuple_size<H>::value * 8 / 10>
void
tests ()
{
  TestVector<M, typename H::key_type> const test;

  H h;
  assert (h == h);

  std::vector<std::pair<typename H::key_type, typename H::mapped_type> > vals;
  std::cout << "0" << std::flush;
  for (auto i = test.begin (); i != test.end (); ++i)
    {
      auto pair = std::make_pair (*i, *i);
      vals.push_back (pair);
      auto p = h.insert (pair);
      assert (p.first != h.end ());
      assert (p.second);
    }

  assert (h == h);
  assert (h.size () == vals.size ());
  if (h.size () > 0)
    assert (h.begin () != h.end ());
  assert (h.begin () == h.begin ());
  assert (h.end () == h.end ());

  std::cout << "1" << std::flush;
  {
    typename H::iterator it = h.begin ();
    typename H::iterator jt = it;
    typename H::const_iterator kt = it;
    assert (it == jt);
    assert (kt == it);
    ++it;
    ++jt;
    assert (it == jt);
    assert ((size_t)std::distance (h.begin (), h.end ()) == vals.size ());
  }
  std::cout << "2" << std::flush;
  {
    auto h2 = h;
    assert (h2 == h);

    membership_tests (test, h);
    dummy_insert_test (test, h);
    assert (h2 == h);

    membership_tests (test, h2);
    dummy_insert_test (test, h2);
    swap (h, h2);
    assert (h2 == h);

    swap (h, h);
    assert (h2 == h);

    h2.insert (std::make_pair (test.extra (), test.extra ()));
    assert (h2 != h);
    assert (!(h2 == h));
  }

  std::cout << "3" << std::flush;
  {
    H const &h3 = h;
    membership_tests (test, h3);
  }

  std::cout << "4" << std::flush;
  {
    H h4;
    h4.insert (vals.begin (), vals.end ());
    std::cout << "." << std::flush;
    assert (h4 == h);
  }

  std::cout << "5" << std::flush;
  {
    H h5;
    h5.insert (vals.rbegin (), vals.rend ());
    std::cout << "." << std::flush;
    assert (h5 == h);

    std::cout << "." << std::flush;
    h5.clear ();
    assert (h5.begin () == h5.end ());
    assert (h5.find (test.extra ()) == h5.end ());
  }
}

template <int N>
void
testsuite ()
{
  std::cout << "running testsuite for N=" << N << std::flush;

  std::cout << std::endl << " + std::map int->int " << std::flush;
  tests<std::map<int, int>, N> ();

  std::cout << std::endl << " + hashtab int->int default hash " << std::flush;
  tests<hashtab<int, int, N>> ();

  std::cout << std::endl << " + hashtab int->int, hash2 " << std::flush;
  tests<hashtab<int, int, N, hash_int2>> ();

  std::cout << std::endl << " + hashtab int->int, hash3, full " << std::flush;
  tests<hashtab<int, int, N, hash_int3>, N - 1> ();

  std::cout << std::endl << " + hashtab int->int, silly hash " << std::flush;
  tests<hashtab<int, int, N, silly_hash<int> >> ();

  std::cout << std::endl << " + hashtab string->string default hash " << std::flush;
  tests<hashtab<std::string, std::string, N>> ();

  std::cout << std::endl << " + hashtab string->string other hash " << std::flush;
  tests<hashtab<std::string, std::string, N, hash_string>> ();

  std::cout << std::endl << " + hashtab string->string silly hash " << std::flush;
  tests<hashtab<std::string, std::string, N, silly_hash<std::string> > > ();

  std::cout << std::endl;
}

struct D
{
  D ()
  {
    assert (!"This ctor should never be called!");
  }
};

int
main (int argc, char *argv[])
{
  std::cout << "tests for tiny tables" << std::flush;

  std::cout << std::endl << " + hashtab N=1 " << std::flush;
  tests<hashtab<int, int, 1>, 0> ();
  tests<hashtab<std::string, std::string, 1>, 0> ();

  std::cout << std::endl << " + hashtab N=2 " << std::flush;
  tests<hashtab<int, int, 2>, 1> ();
  tests<hashtab<std::string, std::string, 2>, 1> ();

  std::cout << std::endl << " + hashtab N=3 " << std::flush;
  tests<hashtab<int, int, 3>, 2> ();
  tests<hashtab<std::string, std::string, 3>, 2> ();

  {
    hashtab<int, D, 3> _;
  }

  std::cout << std::endl;
  testsuite<5> ();
  testsuite<17> ();
  testsuite<31> ();
  testsuite<61> ();
  testsuite<127> ();
  testsuite<251> ();
  testsuite<509> ();
  testsuite<1021> ();
  testsuite<2039> ();
  testsuite<4093> ();
  testsuite<8191> ();
  testsuite<65521> ();
}
