CC		= g++
CXXFLAGS	= -std=c++0x

.PHONY: all debug clean

all: CXXFLAGS += -O3 -DNDEBUG
all: project3

debug: CXXFLAGS += -g
debug: project3

project3: project3.cpp p2lex.cpp getToken.cpp parser.cpp
	$(CC) $(CXXFLAGS) -o $@ $+

clean:
	rm -rf project3
