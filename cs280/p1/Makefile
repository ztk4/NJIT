CC			= g++
CXXFLAGS	= -std=c++0x -O3

all: main

debug: CXXFLAGS += -DDEBUG -g
debug: main

main: main.cpp
	$(CC) $(CXXFLAGS) -o main main.cpp

clean:
	rm -rf main
