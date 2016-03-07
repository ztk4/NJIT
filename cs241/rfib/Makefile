NAME		= rfib

CC			= g++
CXXFLAGS	= -std=c++11

.PHONY: all debug clean output

all: CXXFLAGS += -O3
all: $(NAME)

debug: CXXFLAGS += -g -DDEBUG
debug: $(NAME)

$(NAME): $(NAME).cpp
	$(CC) $(CXXFLAGS) -o $@ $+

output: output.txt

output.txt: $(NAME).out
	echo "Zachary Kaplan" > $@
	echo "CS241 T 6-9 PM" >> $@
	echo "HW $(NAME)" >> $@
	date "+%-m/%-d/%-y" >> $@
	more $(NAME).cpp $(NAME).out >> $@

$(NAME).out: $(NAME)
	./$(NAME) >> $@

clean:
	rm -rf $(NAME) output.txt $(NAME).out
