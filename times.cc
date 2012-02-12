#include "hash.hh"
#include "slist.hh"

#include <boost/progress.hpp>
#include <cassert>
#include <forward_list>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include "tests.hh"

template<template<size_t N> class Hc>
void
test_hash ()
{
  enum { N = 65521, M = 52415 };
  //enum { N = 65521, M = 52415 };
  //enum { N = 87359, M = 80000 };
  TestVector<M, int> const test;

  std::vector<std::pair<int, int> > vals;
  for (auto i = test.begin (); i != test.end (); ++i)
    vals.push_back (std::make_pair (*i, *i));

  typedef typename Hc<N>::type H;
  std::cout << "Measuring " << typeid (H).name () << std::endl;

  {
    boost::progress_timer t0;
    H h1;
    {
      std::cout << " + h1.insert (vals.begin (), vals.end ()): " << std::flush;
      boost::progress_timer t;
      for (int i = 0; i < 1000; ++i)
	{
	  h1.clear ();
	  h1.insert (vals.begin (), vals.end ());
	}
    }

    H h2;
    {
      std::cout << " + h2.insert (vals.rbegin (), vals.rend ()): " << std::flush;
      boost::progress_timer t;
      for (int i = 0; i < 1000; ++i)
	{
	  h2.clear ();
	  h2.insert (vals.rbegin (), vals.rend ());
	}
    }

    {
      boost::progress_timer t;
      std::cout << " + h1 == h2: " << std::flush;
      for (int i = 0; i < 1000; ++i)
	{
	  h2 == h1;
	}
    }
  }
}

template<template <size_t N> class Hc>
void
test_slist ()
{
  enum { N = 32000, M = N - 1 };
  TestVector<M, int> const test;

  std::vector<int> vals (test.begin (), test.end ());

  typedef typename Hc<N>::type H;
  std::cout << "Measuring " << typeid (H).name () << std::endl;

  {
    boost::progress_timer t0;
    H h1;
    {
      std::cout << " + h1 = H (vals.begin (), vals.end ()) :" << std::flush;
      boost::progress_timer t;
      for (int i = 0; i < 1000; ++i)
	h1 = H (vals.begin (), vals.end ());
    }

    H h2;
    {
      std::cout << " + for i in vals.rbegin ()..vals.rend (): h2.push_front(*i) :" << std::flush;
      boost::progress_timer t;
      for (int i = 0; i < 1000; ++i)
	{
	  h2.clear ();
	  for (auto it = vals.rbegin (); it != vals.rend (); ++it)
	    h2.push_front (*it);
	}
    }

    {
      std::cout << " + h1 == h2 :" << std::flush;
      boost::progress_timer t;
      for (int i = 0; i < 1000; ++i)
	{
	  h2 == h1;
	}
    }
  }
}

template<size_t N>
struct slistC
{
  typedef slist<int, N> type;
};

template<size_t N>
struct fwdlistC
{
  typedef std::forward_list<int> type;
};

template<size_t N>
struct listC
{
  typedef std::list<int> type;
};

template<size_t N>
struct hashtabC
{
  typedef hashtab<int, int, N> type;
};

template<size_t N>
struct mapC
{
  typedef std::map<int, int> type;
};

template<size_t N>
struct unomapC
{
  typedef std::unordered_map<int, int> type;
};


int
main(int argc, char *argv[])
{
  for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];
      if (arg == "hash")
	{
	  test_hash<hashtabC> ();
	  test_hash<mapC> ();
	  test_hash<unomapC> ();
	}
      else if (arg == "slist")
	{
	  test_slist<slistC> ();
	  test_slist<fwdlistC> ();
	  test_slist<listC> ();
	}
      else
	std::cout << "Unknown test " << arg << std::endl;
    }
  return 0;
}
