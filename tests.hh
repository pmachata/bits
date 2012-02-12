#include <vector>
#include <algorithm>

template <int N, class T>
struct TestVector;

template <int N>
struct TestVector<N, int>
  : public std::vector<int>
{
  int i;

  TestVector ()
    : i (728)
  {
    for (int j = 0; j < N; ++j)
      {
	push_back (i);
	i += 17;
      }
  }

  int
  extra () const
  {
    return i;
  }
};

namespace
{
  template <int M> struct slen {
    enum { value = slen<M / 2>::value + 1 };
  };
  template <> struct slen<1> {
    enum { value = 0 };
  };
  template <> struct slen<0> {
    enum { value = 0 };
  };
}

template <int N>
struct TestVector<N, std::string>
  : public std::vector<std::string>
{
  std::string key;

  TestVector ()
  {
    int c = ' ';
    for (int i = 0; i < N; ++i)
      {
	if (std::find (begin (), end (), key) == end ())
	  push_back (key);

	c = (c + 17) % 127;
	if (c < ' ')
	  c += ' ';
	key += char (c);
	if (key.size () > ::slen<N>::value)
	  key = key.substr (1, key.size ());
      }
  }

  std::string
  extra () const
  {
    return key + 'x';
  }
};
