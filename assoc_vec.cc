/*
 * Test suite for associative container on top of std::vector.
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

#include "assoc_vec.hh"
#include "tests.hh"

#include <vector>
#include <algorithm>
#include <map>
#include <cassert>
#include <iostream>

template <class T, class U>
void
dummy_insert_test (T const &test, U &h)
{
  std::cout << "." << std::flush;
  for (auto i = test.begin (); i != test.end (); ++i)
    {
      auto p = h.insert (std::make_pair (*i, *i));
      assert (p.first != h.end ());
      assert (!p.second);
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

  std::cout << std::endl << " + assoc_vec int->int " << std::flush;
  tests<assoc_vec<int, int>, N> ();

  std::cout << std::endl << " + assoc_vec string->string " << std::flush;
  tests<assoc_vec<std::string, std::string>, N> ();

  std::cout << std::endl;
}

int
main (int argc, char *argv[])
{
  std::cout << "tests for tiny tables" << std::flush;

  std::cout << std::endl << " + assoc_vec N=0 " << std::flush;
  tests<assoc_vec<int, int>, 0> ();
  tests<assoc_vec<std::string, std::string>, 0> ();

  std::cout << std::endl << " + assoc_vec N=1 " << std::flush;
  tests<assoc_vec<int, int>, 1> ();
  tests<assoc_vec<std::string, std::string>, 1> ();

  std::cout << std::endl << " + assoc_vec N=2 " << std::flush;
  tests<assoc_vec<int, int>, 2> ();
  tests<assoc_vec<std::string, std::string>, 2> ();

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
