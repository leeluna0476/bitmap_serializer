.DEFAULT_GOAL=all

NAME=drawing_board
CXX=c++
CXXFLAGS=-Wall -Wextra -Werror --std=c++98 -Wpedantic
CPPFLAGS=-MMD -MP

RM=rm -fr

SRCS=\
	 bmp_serializer.cpp \
	 env.cpp \
	 main.cpp

OBJS=$(SRCS:.cpp=.o)
DEPS=$(OBJS:.o=.d)
-include $(DEPS)

ifeq ($(MAKECMDGOALS),debug)
	CXXFLAGS+=-g3 -fsanitize=address
endif

%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

all: $(NAME)

$(NAME): $(OBJS)
	$(LINK.cpp) $(OUTPUT_OPTION) $(OBJS)

debug: all

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)

re: fclean
	$(MAKE) all

.PHONY: all clean fclean re
