CC	= gcc
CFLAGS	= 
LDFLAGS = -lm
TARGET	= hw7

.PHONY: all debug clean

all: CFLAGS += -O3 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -g
debug: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

clean:
	rm -rf $(TARGET) *.o
