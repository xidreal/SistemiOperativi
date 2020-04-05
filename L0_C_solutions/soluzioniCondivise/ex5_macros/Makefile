
NAME=macroSquare
CFLAGS=-Wall
INCLUDES=-I./inc

SRCS=macroSquare.c
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

.c.o:
	@echo "Compiling: "$<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	@rm -f *.o $(NAME)
	@echo "Removed object files and executable..."
