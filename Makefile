CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++17

NAME = webserv

SRC = srcs/main.cpp srcs/Server.cpp srcs/Location.cpp srcs/InitConfig.cpp srcs/ParseConfig.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re