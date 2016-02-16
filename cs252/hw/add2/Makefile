NAME		= add2

CC			= arm-none-linux-gnueabi-gcc
EXEC		= arm-none-linux-gnueabi-qemu
EXECFLAGS	= -L /usr/arm-linux-gnueabi/
FLAGS		= 
CFLAGS		= -std=c90 -O3

.PHONY: all debug clean output

all: $(NAME)

debug: FLAGS += -DDEBUG -g
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
	echo "HW 2 - Add2" >> $@
	echo "2/16/16" >> $@
	more driver.c $(NAME).s >> $@
	echo "Execution Output:" >> $@
	$(EXEC) $(EXECFLAGS) $(NAME) >> $@

clean:
	rm -rf $(NAME) *.o output.txt
