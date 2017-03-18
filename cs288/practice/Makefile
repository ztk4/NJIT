MPICC	= mpicc
CC	= gcc
CFLAGS	= 
LDFLAGS = -lm
TARGETS = split mpi

.PHONY: all debug clean

all: CFLAGS += -O3 -DNDEBUG
all: $(TARGETS)

debug: CFLAGS += -g
debug: $(TARGETS)

split: split.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

mpi: mpi.c
	$(MPICC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

clean:
	rm -rf $(TARGETS) *.o
