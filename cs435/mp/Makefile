# Zachary Kaplan [3570] CS435-001 MP
# ALL
# 9/27/17

CXX       = g++
CPPFLAGS += 
CXXFLAGS += -std=c++11 -O3 -Wall -Wextra
LDFLAGS  += -lm

TARGETS = henc_3570 hdec_3570 mplexicon_3570

MP1_HEADERS = $(addsuffix _3570.h, utils logging bit_stream priority_queue prefix_cache)
MP2_HEADERS = $(addsuffix _3570.h, utils logging hash_table lexicon)

.PHONY: all debug clean opt targets

all: opt

opt: CPPFLAGS += -DNDEBUG
opt: targets

debug: CXXFLAGS += -ggdb
debug: targets

clean:
	rm -rf *.o $(TARGETS)

targets: $(TARGETS)

# MP1

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

# MP2

mplexicon_3570: mplexicon_3570.o lexicon_3570.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

mplexicon_3570.o: $(MP2_HEADERS) mplexicon_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c mplexicon_3570.cc -o $@

lexicon_3570.o: $(MP2_HEADERS) lexicon_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c lexicon_3570.cc -o $@
