NAME		= codexion
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread -Iinclude
SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= include

# Find all .c files recursively in src/
SRCS		:= $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS		:= $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/codexion.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re