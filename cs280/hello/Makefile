CC			= g++
CXXFLAGS	= -std=c++11 -O3

all: main

debug: CXXFLAGS += -DDEBUG -g
debug: main

main: main.cpp
	$(CC) $(CXXFLAGS) -o main main.cpp

clean:
	rm -rf main
