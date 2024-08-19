# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/12 20:53:34 by isromero          #+#    #+#              #
#    Updated: 2024/06/23 11:31:05 by isromero         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -O3 -std=c++98 -I include
RM = rm -f

SRCSDIR = src
OBJSDIR = objs

SRCS = main.cpp Server.cpp Socket.cpp Request.cpp Response.cpp utils.cpp ServerConfig.cpp
OBJS = $(addprefix $(OBJSDIR)/, $(SRCS:.cpp=.o))

# Colors
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
BLUE = \033[0;34m
MAGENTA = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[0;37m
NC = \033[0m # No Color

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@printf "$(GREEN)✔ $(NAME) has been created.$(NC)\n"

$(OBJSDIR)/%.o: $(SRCSDIR)/%.cpp | $(OBJSDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJSDIR):
	mkdir -p $(OBJSDIR)

clean:
	@if [ -d "$(OBJSDIR)" ]; then \
        $(RM) -r $(OBJSDIR) && \
        printf "$(YELLOW)✔ Objects have been removed.$(NC)\n"; \
    else \
        printf "$(YELLOW)✘ No objects to remove.$(NC)\n"; \
    fi

fclean:
	@if [ -d "$(OBJSDIR)" ] || [ -f "$(NAME)" ]; then \
        $(RM) -r $(OBJSDIR) && \
        $(RM) $(NAME) && \
        printf "$(RED)✔ $(NAME) and objects have been removed.$(NC)\n"; \
    else \
        printf "$(RED)✘ Nothing to clean.$(NC)\n"; \
    fi

re: fclean all
