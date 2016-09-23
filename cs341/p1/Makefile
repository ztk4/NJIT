CC		= g++
CXXFLAGS	= -std=c++11

.PHONY: all debug clean dfa

all: CXXFLAGS += -O3 -DNDEBUG
all: project1 dfa

debug: CXXFLAGS += -g
debug: project1

project1: p1_16f_ztk4.cpp
	$(CC) $(CXXFLAGS) -o $@ $+

dfa: dfa.pdf

dfa.pdf: dfa.svg
	rsvg-convert -f pdf -o $@ $+

dfa.svg: dfa.dot
	dot -Tsvg -o $@ $+

clean:
	rm -rf project1 *.o
