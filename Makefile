all: hash slist assoc_vec times

hash: hash.cc hash.hh tests.hh
slist: slist.cc slist.hh forward_vec.hh tests.hh
assoc_vec: assoc_vec.cc assoc_vec.hh tests.hh
rbtree: rbtree.cc rbtree.hh tests.hh
prime_iterator: prime_iterator.cc prime_iterator.hh
times: times.cc $(wildcard *.hh)
prime_iterator times hash slist assoc_vec rbtree: CXXFLAGS = -std=c++0x -Wall -g -O2
