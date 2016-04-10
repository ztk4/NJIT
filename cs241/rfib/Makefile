NAME		= rfib

CC		= g++
CXXFLAGS	= -std=c++11

.PHONY: all debug clean output

all: CXXFLAGS += -O3
all: $(NAME)

debug: CXXFLAGS += -g -DDEBUG
debug: $(NAME)

$(NAME): $(NAME).cpp
	$(CC) $(CXXFLAGS) -o $@ $+

output: $(NAME).out

$(NAME).out: $(NAME)
	@echo "Generating values up to fib(50) (ie before compute time excedes 2 minutes)"
	@echo "Writing output to file and stdin..."
	./$(NAME) 50 | tee $@
	@echo "Done"

clean:
	rm -rf $(NAME)
