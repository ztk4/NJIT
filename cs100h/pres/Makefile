CC      = gcc
CFLAGS  = -std=c99
LDFLAGS = -lssl -lcrypto

all: genhash overflow

genhash: genhash.c
	$(CC) $(CFLAGS) -O3 -o genhash genhash.c $(LDFLAGS)

overflow: overflow.c
	$(CC) $(CFLAGS) -g -fno-stack-protector -o overflow overflow.c $(LDFLAGS)

clean:
	rm genhash overflow
