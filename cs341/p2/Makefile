CC		= g++
CXXFLAGS	= -std=c++11 -Wall -Wpedantic

.PHONY: all debug clean pda

all: CXXFLAGS += -O3 -DNDEBUG
all: project2 pda

debug: CXXFLAGS += -g
debug: project2

project2: p2_17s_ztk4.cpp
	$(CC) $(CXXFLAGS) -o $@ $+

pda: pda.pdf

pda.pdf: pda.svg
	rsvg-convert -f pdf -o $@ $+

pda.svg: pda.dot
	dot -Tsvg -o $@ $+

clean:
	rm -rf project2 *.o pda.svg pda.pdf
