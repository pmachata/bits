#include "slist.hh"
#include "forward_vec.hh"
#include "tests.hh"

#include <iostream>
#include <cstddef>
#include <forward_list>
#include <cassert>

template <class H, int M>
void
tests (char const *msg = "")
{
  std::cout << std::endl << " + " << typeid (H).name ()
	    << " " << typeid (typename H::value_type).name ()
	    << msg << "\t" << std::flush;

  H h;
  assert (h == h);
  assert (h.begin () == h.end ());

  TestVector<M, typename H::value_type> const test;
  std::vector<typename H::value_type> const vals (test.begin (), test.end ());

  for (size_t loops = 0; ; ++loops)
    {
      std::cout << loops << std::flush;
      assert (h.empty ());
      for (auto i = vals.begin (); i != vals.end (); ++i)
	{
	  h.push_front (*i);
	  assert (h.front () == *i);
	  assert (!h.empty ());
	}

      std::cout << "." << std::flush;
      assert ((size_t)std::distance (h.begin (), h.end ()) == test.size ());

      std::cout << "." << std::flush;
      {
	auto it = vals.rbegin ();
	auto jt = h.begin ();
	for (; it != vals.rend () && jt != h.end (); ++it, ++jt)
	  assert (*it == *jt);
	assert ((it == vals.rend ()) == (jt == h.end ()));
      }

      std::cout << "." << std::flush;
      assert (h == h);
      if (test.size () > 0)
	assert (h.begin () != h.end ());
      assert (h.begin () == h.begin ());
      assert (h.end () == h.end ());
      assert ((size_t)std::distance (h.begin (), h.end ()) == vals.size ());

      std::cout << "." << std::flush;
      {
	H h2 (vals.rbegin (), vals.rend ());
	assert (std::distance (h2.begin (), h2.end ())
		== std::distance (h.begin (), h.end ()));
	assert (h == h2);

	H h3;
	if (M > 0)
	    h3.push_front (vals[0]);
	h3.swap (h2);
	assert (h == h3);
	if (M > 0)
	  assert (std::distance (h2.begin (), h2.end ()) == 1);
      }

      {
	H *h2 = new H (h);
	for (size_t i = 1; i < M; ++i)
	  {
	    if (i % 1000 == 0)
	      std::cout << "." << std::flush;
	    std::vector<typename H::value_type> save;
	    for (size_t j = 0; j < i; ++j)
	      {
		assert (!h2->empty ());
		typename H::value_type v = h2->front ();
		save.push_back (v);
		h2->pop_front ();
	      }
	    while (!save.empty ())
	      {
		h2->push_front (save.back ());
		save.pop_back ();
	      }
	    assert (*h2 == h);
	  }
	delete h2;
      }

      std::cout << "2" << std::flush;
      {
	enum { K = M / 2 };
	H h2 = h;
	h2.resize (K);
	assert (std::distance (h2.begin (), h2.end ()) == K);

	auto jt = vals.rbegin ();
	for (auto it = h2.begin (); it != h2.end (); )
	  assert (*it++ == *jt++);

	enum { L = M * 2 / 3 };
	h2.resize (L, test.extra ());
	assert (std::distance (h2.begin (), h2.end ()) == L);
	auto it = h2.begin ();
	std::advance (it, K);
	for (size_t i = K; i < L; ++i)
	  assert (*it++ == test.extra ());
      }

      if (M > 1)
	{
	  std::cout << "." << std::flush;
	  H h2;
	  h2.push_front (vals.front ());
	  h2.insert_after (h2.begin (), test.extra ());
	  auto it = h2.begin ();
	  assert (*it++ == vals.front ());
	  assert (*it++ == test.extra ());
	}

      switch (loops)
	{
	case 0:
	  std::cout << "E" << std::flush;
	  if (M > 0)
	    {
	      h.erase_after (h.begin (), h.end ());
	      h.pop_front ();
	    }
	  continue;

	case 1:
	  std::cout << "C" << std::flush;
	  h.clear ();
	  continue;

	case 2:
	  std::cout << "P" << std::flush;
	  while (h.begin () != h.end ())
	    h.pop_front ();
	  continue;
	};

      std::cout << "X" << std::flush;
      break;
    }
}

struct C
{
  // this is for testing that empty slist doesn't contain objects
  C () { assert (false); }
};

struct D
{
  int *i;
  D (int *ii) : i (ii) {}
  D (D const &other) : i (other.i) {}
  ~D () { ++*i; }
};

template <template <class, int> class Ct, int N>
void
custom_testsuite ()
{
  tests<typename Ct<int, N>::type, N - 1> ();
  tests<typename Ct<int, N>::type, N> (": full");
  tests<typename Ct<std::string, N>::type, N - 1> ();

  std::cout << std::endl << " + object store tests " << std::flush;
  {
    // test that ctor is not called for empty list
    typename Ct<C, N>::type slist;
  }
  {
    // test that dtors are called as required
    int ct = 0;
    {
      typename Ct<D, N>::type slist;
      {
	D d (&ct);
	slist.push_front (d);
      }
      assert (ct == 1); // "D d" destroyed
    }
    assert (ct == 2); // the copy internal to slist destroyed
  }
  std::cout << std::endl;
}

template <template <class, int> class Ct, int N>
void
test_overfill ()
{
  // test that overfilling slist throws
  int ok = 0;
  try
    {
      tests<typename Ct<int, N>::type, N + 1> ();
    }
  catch (std::bad_alloc const &e)
    {
      ok = 1;
    }
  assert (ok);
}

template <class T, int N>
struct slistC
{
  typedef slist<T, N> type;
};

template <class T, int N>
struct fwdvecC
{
  typedef forward_vec<T> type;
};

template <int N>
void
testsuite ()
{
  std::cout << "running testsuite for N=" << N << std::flush;
  tests<std::forward_list<int>, N - 1> ();
  custom_testsuite<slistC, N> ();
  test_overfill<slistC, N> ();
  custom_testsuite<fwdvecC, N> ();
}

int
main(int argc, char *argv[])
{
  if (true)
    {
      testsuite<1> ();
      testsuite<4> ();
      testsuite<16> ();
      testsuite<64> ();
      testsuite<256> ();
      testsuite<1024> ();
      testsuite<4096> ();
      testsuite<16384> ();
    }
  else
    {
      enum { N = 110, M = N - 1 };
      typedef slist<std::string, N> H;
      TestVector<M, typename H::value_type> v;
      std::cout << std::distance (v.begin (), v.end ()) << std::endl;
      H h (v.begin (), v.end ());
      H *h2 = new H (h);
      size_t i = M;
      std::vector<typename H::value_type> save;
      for (size_t j = 0; j < i; ++j)
	{
	  assert (!h2->empty ());
	  typename H::value_type v = h2->front ();
	  save.push_back (v);
	  h2->pop_front ();
	}
      while (!save.empty ())
	{
	  h2->push_front (save.back ());
	  save.pop_back ();
	}
      assert (*h2 == h);
      delete h2;
    }
  return 0;
}
