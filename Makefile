all: hash slist assoc_vec times

hash: hash.cc hash.hh tests.hh
slist: slist.cc slist.hh forward_vec.hh tests.hh
assoc_vec: assoc_vec.cc assoc_vec.hh tests.hh
times: times.cc $(wildcard *.hh)
times hash slist assoc_vec: CXXFLAGS = -std=c++0x -Wall -g -O2
