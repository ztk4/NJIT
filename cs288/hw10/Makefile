CC	= mpicc
CFLAGS	= 
LDFLAGS = -lm
TYPES	= p2p collective
TARGETS	= $(addprefix dot_, $(TYPES))

.PHONY: all debug clean run

all: CFLAGS += -O3 -DNDEBUG
all: $(TARGETS)

debug: CFLAGS += -g
debug: $(TARGETS)

$(TARGETS): %: %.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

clean:
	rm -rf $(TARGETS) *.o
