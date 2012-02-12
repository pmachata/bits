#include "slist.hh"
#include "tests.hh"

#include <iostream>
#include <cstddef>
#include <forward_list>
#include <cassert>

template <class H, int M = std::tuple_size<H>::value - 1>
void
tests ()
{
  H h;
  assert (h == h);
  assert (h.begin () == h.end ());

  TestVector<M, typename H::value_type> const test;
  std::vector<typename H::value_type> const vals (test.begin (), test.end ());

  for (size_t loops = 0; ; ++loops)
    {
      std::cout << loops << std::flush;
      for (auto i = vals.begin (); i != vals.end (); ++i)
	h.push_front (*i);

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

      break;
    }
}

template <int N>
void
testsuite ()
{
  std::cout << "running testsuite for N=" << N << std::flush;

  std::cout << std::endl << " + std::forward_list int " << std::flush;
  tests<std::forward_list<int>, N - 1> ();

  std::cout << std::endl << " + slist int " << std::flush;
  tests<slist<int, N>> ();

  std::cout << std::endl;
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
      slist<int, 10> x;
      x.push_front (1);
      x.push_front (2);
      std::cout << std::distance (x.begin (), x.end ()) << std::endl;
    }
  return 0;
}
