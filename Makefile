NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC = src/main.c \
	  src/parse.c \
	  src/coder.c \
	  src/utils.c \
	  src/init.c \
	  src/logging.c \
	  src/time.c \
	  src/dongle.c \
	  src/state.c \
	  src/monitor.c \
	  src/heap.c \
	  src/scheduler.c

OBJ = $(SRC:.c=.o)

INCLUDES = -Iinclude

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re