CC		= g++
CXXFLAGS	= -std=c++11

.PHONY: all debug clean

all: CXXFLAGS += -O3 -DNDEBUG
all: project1

debug: CXXFLAGS += -g
debug: project1

project1: p1_16f_ztk4.cpp
	$(CC) $(CXXFLAGS) -o $@ $+

clean:
	rm -rf project1 *.o
