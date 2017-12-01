GTEST_DIR = $(HOME)/googletest/googletest
GMOCK_DIR = $(HOME)/googletest/googlemock

# Compiler to use
CXX = g++

# Flags passed to the preprocessor.
# Set Google Test and Google Mock's header directories as system
# directories, such that the compiler doesn't generate warnings in
# these headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -std=c++11 -O3 -Wall -Wextra -pthread

# Flags passed to the C++ linker.
LDFLAGS += -lpthread

# All tests produced by this Makefile.  Remember to add new tests you
# create to the list.
TESTS = priority_queue_test bit_stream_test prefix_cache_test hash_table_test lexicon_test

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# All Google Mock headers. Note that all Google Test headers are
# included here too, as they are #included by Google Mock headers.
# Usually you shouldn't change this definition.	
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

# House-keeping build targets.

.PHONY: test all debug check clean

all: CPPFLAGS+= -DNDEBUG
all: test

debug: CXXFLAGS+= -ggdb
debug: test

test: $(TESTS)

check: all
	@for test in *_test; do echo "Running $${test}..."; ./$${test}; done

clean:
	rm -f $(TESTS) gmock.a gmock_main.a gtest.a gtest_main.a *.o

# Builds gmock.a and gmock_main.a.  These libraries contain both
# Google Mock and Google Test.  A test should link with either gmock.a
# or gmock_main.a, depending on whether it defines its own main()
# function.  It's fine if your test only uses features from Google
# Test (and not Google Mock).

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

# For simplicity and to avoid depending on implementation details of
# Google Mock and Google Test, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Mock and
# Google Test compile fast and for ordinary users their source rarely
# changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
	    -c $(GTEST_DIR)/src/gtest_main.cc

gmock-all.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc

gmock_main.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

gmock.a : gmock-all.o gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gmock_main.a : gmock-all.o gtest-all.o gmock_main.o
	$(AR) $(ARFLAGS) $@ $^

# GMOCK TESTS

priority_queue_test.o: ../utils_3570.h ../priority_queue_3570.h priority_queue_test.cc
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
		-c priority_queue_test.cc
priority_queue_test: priority_queue_test.o gmock_main.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

bit_stream_test.o: ../utils_3570.h ../bit_stream_3570.h bit_stream_test.cc
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
		-c bit_stream_test.cc
bit_stream_test: bit_stream_test.o bit_stream.o gmock_main.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

prefix_cache_test.o: ../utils_3570.h ../bit_stream_3570.h ../prefix_cache_3570.h prefix_cache_test.cc
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
		-c prefix_cache_test.cc
prefix_cache_test: prefix_cache_test.o prefix_cache.o bit_stream.o gmock_main.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# GTEST TESTS

hash_table_test.o: ../utils_3570.h ../hash_table_3570.h hash_table_test.cc
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) \
		-c hash_table_test.cc
hash_table_test: hash_table_test.o gtest_main.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

lexicon_test.o: ../utils_3570.h ../lexicon_3570.h lexicon_test.cc
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) \
		-c lexicon_test.cc
lexicon_test: lexicon.o lexicon_test.o gtest_main.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# INTERMEDIATE TARGETS

bit_stream.o: ../utils_3570.h ../bit_stream_3570.h ../bit_stream_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../bit_stream_3570.cc -o $@

prefix_cache.o: ../utils_3570.h ../prefix_cache_3570.h ../bit_stream_3570.h ../priority_queue_3570.h ../prefix_cache_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../prefix_cache_3570.cc -o $@

lexicon.o: ../utils_3570.h ../hash_table_3570.h ../lexicon_3570.h ../lexicon_3570.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../lexicon_3570.cc -o $@
