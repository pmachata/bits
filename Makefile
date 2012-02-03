all: hash

hash: hash.cc hash.hh
hash: CXXFLAGS = -std=c++0x -Wall -g -O2
