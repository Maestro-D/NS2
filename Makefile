SRC = 		src/main.cpp

NAME = 		SSHProxy

OBJ =		$(SRC:.cpp=.o)

CC =		g++ -g

RM =		rm -f

all:		$(NAME)

$(NAME):	$(OBJ)
			$(CC) -o $(NAME) $(OBJ) -lssh

src/%.o:	src/%.cpp
			$(CC) -o $@ -c $< -Iheaders

clean:
			$(RM) $(OBJ)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all