CXX       = g++
CPPFLAGS += 
CXXFLAGS += -O3 -Wall -Wextra
LDFLAGS  += -lm

# TODO: Fill in top level targets
TARGETS =

.PHONY: all debug opt targets

all: opt

opt: CPPFLAGS += -DNDEBUG
opt: targets

debug: CXXFLAGS += -g
debug: targets

targets: $(TARGETS)
