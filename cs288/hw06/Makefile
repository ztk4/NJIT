CC	= gcc
CFLAGS	= 
LDFLAGS = -lm
MODES 	= SELECTION MERGE INT_RADIX FLOAT_RADIX

.PHONY: all debug clean

all: CFLAGS += -O3 -DNDEBUG
all: $(MODES)

debug: CFLAGS += -g
debug: $(MODES)

$(MODES): hw6.c
	$(CC) $(CFLAGS) -D$@ -o $@ $+ $(LDFLAGS)

clean:
	rm -rf $(MODES) *.o
