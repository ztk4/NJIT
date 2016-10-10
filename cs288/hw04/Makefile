CC	= gcc
CFLAGS	= 
TARGET	= memmap

.PHONY: all debug clean

all: CFLAGS += -O3 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -g
debug: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -rf $(TARGET) *.o
