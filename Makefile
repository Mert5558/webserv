CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++17

NAME = webserv

OBJDIR = obj
SRC = srcs/main.cpp srcs/Location.cpp srcs/InitConfig.cpp srcs/ParseConfig.cpp srcs/ParseHttp.cpp srcs/Server.cpp
OBJ = $(SRC:srcs/%.cpp=$(OBJDIR)/%.o)

all: $(OBJDIR) $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJ)

$(OBJDIR)/%.o: srcs/%.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all


# CPP = c++
# CPPFLAGS = -Wall -Werror -Wextra -std=c++17

# NAME = webserv

# SRC = srcs/main.cpp srcs/Location.cpp srcs/InitConfig.cpp srcs/ParseConfig.cpp srcs/ParseHttp.cpp srcs/Server.cpp srcs/httpResponse.cpp
# OBJ = $(SRC:.cpp=.o)

# all: $(NAME)

# $(NAME): $(OBJ)
# 	$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJ)

# %.o: %.cpp
# 	$(CPP) $(CPPFLAGS) -c $< -o $@

# clean:
# 	rm -rf $(OBJ)

# fclean: clean
# 	rm -rf $(NAME)

# re: fclean all

# .PHONY: all clean fclean re