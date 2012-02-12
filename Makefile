all: hash slist

hash: hash.cc hash.hh tests.hh
slist: slist.cc slist.hh tests.hh
hash slist: CXXFLAGS = -std=c++0x -Wall -g -O2
