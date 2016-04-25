NAME		= caesar

CC		= arm-none-linux-gnueabi-gcc
EXEC		= arm-none-linux-gnueabi-qemu
EXECFLAGS	= -L /usr/arm-linux-gnueabi/
FLAGS		= 
CFLAGS		= -std=c90

.PHONY: all debug clean output

all: FLAGS += -DNDEBUG -O3
all: $(NAME)

debug: FLAGS += -g
debug: $(NAME)

$(NAME): $(NAME).o driver.o
	$(CC) $(FLAGS) -o $@ $+ $(LDFLAGS)

$(NAME).o: $(NAME).s
	$(CC) $(FLAGS) -c -o $@ $+

driver.o: driver.c
	$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $+

output: output.txt

output.txt: $(NAME)
	echo "Zachary Kaplan" > $@
	echo "CS252 TR 1:00 - 2:30" >> $@
	echo "HW $(NAME)" >> $@
	date "+%-m/%-d/%-y" >> $@
	more driver.c $(NAME).s >> $@
	python -c "print '='*80" >> $@
	echo "Execution Output:" >> $@
	$(EXEC) $(EXECFLAGS) $(NAME) >> $@

clean:
	rm -rf $(NAME) *.o output.txt
