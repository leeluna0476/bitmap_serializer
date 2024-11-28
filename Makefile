.DEFAULT_GOAL=all

NAME=drawing_board.a
CXX=c++
CXXFLAGS=-Wall -Wextra -Werror --std=c++98 -Wpedantic
CPPFLAGS=-MMD -MP -MF $(DEPS_DIR)/$*.d
AR=ar
ARFLAGS=rc

RM=rm -fr

SRCS=\
	 bmp_serializer.cpp \
	 env.cpp \


OBJS_DIR=.objs
OBJS=$(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))

DEPS_DIR=.deps
DEPS=$(addprefix $(DEPS_DIR)/, $(SRCS:.cpp=.d))
-include $(DEPS)

ifeq ($(MAKECMDGOALS),debug)
	CXXFLAGS+=-g3 -fsanitize=address
endif

all : $(NAME)

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $?

$(OBJS_DIR)/%.o: %.cpp | $(OBJS_DIR) $(DEPS_DIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

debug: all

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(OBJS_DIR) $(DEPS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean
	$(MAKE) all

.PHONY: all clean fclean re
