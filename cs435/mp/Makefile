CXX       = g++
CPPFLAGS += 
CXXFLAGS += -std=c++11 -O3 -Wall -Wextra
LDFLAGS  += -lm

# TODO: Fill in top level targets
TARGETS = henc_3570 hdec_3570

MP1_HEADERS = $(addsuffix _3570.h, utils logging bit_stream priority_queue prefix_cache)

.PHONY: all debug clean opt targets

all: opt

opt: CPPFLAGS += -DNDEBUG
opt: targets

debug: CXXFLAGS += -ggdb
debug: targets

clean:
	rm *.o $(TARGETS)

targets: $(TARGETS)

henc_3570: henc_3570.o bit_stream_3570.o prefix_cache_3570.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

henc_3570.o: $(MP1_HEADERS) henc_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c henc_3570.cc -o $@

hdec_3570: hdec_3570.o bit_stream_3570.o prefix_cache_3570.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

hdec_3570.o: $(MP1_HEADERS) hdec_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c hdec_3570.cc -o $@

bit_stream_3570.o: $(MP1_HEADERS) bit_stream_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c bit_stream_3570.cc -o $@

prefix_cache_3570.o: $(MP1_HEADERS) prefix_cache_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c prefix_cache_3570.cc -o $@
