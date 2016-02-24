CC			= g++
CXXFLAGS	= -std=c++0x -O3

.PHONY: all debug clean

all: project2

debug: CXXFLAGS += -g -DDEBUG
debug: project2

project2: p2lex.h p2lex.cpp getToken.cpp project2.cpp
	$(CC) $(CXXFLAGS) -o $@ $+

clean:
	rm -rf project2
