all: hash slist times

hash: hash.cc hash.hh tests.hh
slist: slist.cc slist.hh tests.hh
times: times.cc $(wildcard *.hh)
times hash slist: CXXFLAGS = -std=c++0x -Wall -g -O2
