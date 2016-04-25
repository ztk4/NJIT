CC		= g++
CXXFLAGS	= -std=c++0x

.PHONY: all debug clean

all: CXXFLAGS += -O3 -DNDEBUG
all: project4

debug: CXXFLAGS += -g
debug: project4

project4: p2lex.o getToken.o parser.o project4.o Value.o
	$(CC) $(CXXFLAGS) -o $@ $+

clean:
	rm -rf project4 *.o
