NAME		= add2

CC			= arm-none-linux-gnueabi-gcc
FLAGS		= 
CFLAGS		= -std=c90 -O3

.PHONY: all debug clean

all: $(NAME)

debug: FLAGS += -DDEBUG -g
debug: $(NAME)

$(NAME): $(NAME).o driver.o
	$(CC) $(FLAGS) -o $@ $+ $(LDFLAGS)

$(NAME).o: $(NAME).s
	$(CC) $(FLAGS) -c -o $@ $+

driver.o: driver.c
	$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $+

clean:
	rm -rf $(NAME) *.o
