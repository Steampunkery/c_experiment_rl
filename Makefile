# @author   clemedon (Clément Vidon)
####################################### BEG_5 ####

NAME        := roguelike

#------------------------------------------------#
#   INGREDIENTS                                  #
#------------------------------------------------#
# LIBS        libraries to be used
# LIBS_TARGET libraries to be built
#
# INCS        header file locations
#
# SRC_DIR     source directory
# SRCS        source files
#
# BUILD_DIR   build directory
# OBJS        object files
# DEPS        dependency files
#
# CC          compiler
# CFLAGS      compiler flags
# CPPFLAGS    preprocessor flags
# LDFLAGS     linker flags
# LDLIBS      libraries name

LIBS        := m uncursed glib-2.0 dijkstra rlsmenu flecs sockui
LIBS_TARGET := lib/rlsmenu/librlsmenu.a lib/flecs/libflecs.a \
			   lib/DijkstraMap/libdijkstra.a lib/sockui/libsockui.a

ROOT_DIR    := $(shell realpath .)
INCS        := include lib/rlsmenu/ lib/flecs/include lib/DijkstraMap/include lib/sockui
INCS        := $(INCS:%=$(ROOT_DIR)/%)

SRC_DIR     := $(shell realpath src)
SRCS        := ai.c component.c gui.c input.c item.c log.c main.c map.c \
			   monster.c observer.c player.c religion.c render.c systems.c \
			   socket_menu.c prefab.c arena.c
SRCS        := $(SRCS:%=$(SRC_DIR)/%)

BUILD_DIR   := $(shell realpath build)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := gcc
CFLAGS		:= -g3 $(shell pkg-config --cflags glib-2.0) -Wall -Wextra -Werror -std=gnu11 -D_GNU_SOURCE
CFLAGS      += $(addprefix -I,$(INCS)) -MMD -MP
LDFLAGS     := $(addprefix -L,$(dir $(LIBS_TARGET)))
LDFLAGS     += -fsanitize=undefined -fno-sanitize-recover
LDLIBS      := $(addprefix -l,$(LIBS))

#------------------------------------------------#
#   UTILITIES                                    #
#------------------------------------------------#
# RM        force remove
# MAKEFLAGS make flags
# DIR_DUP   duplicate directory tree

RM          := rm -f
MAKEFLAGS   += --silent --no-print-directory
DIR_DUP     = mkdir -p $(@D)

#------------------------------------------------#
#   RECIPES                                      #
#------------------------------------------------#
# all       default goal
# $(NAME)   link .o -> archive
# $(LIBS)   build libraries
# %.o       compilation .c -> .o
# clean     remove .o
# fclean    remove .o + binary
# re        remake default goal
# run       run the program
# info      print the default goal recipe

all: $(NAME)

$(NAME): $(OBJS) $(LIBS_TARGET)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)
	$(info CREATED $(NAME))

$(LIBS_TARGET):
	$(MAKE) -C $(@D)
	$(info CREATED $@)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)

-include $(DEPS)

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(NAME)

fclean: clean
	for f in $(dir $(LIBS_TARGET)); do $(MAKE) -C $$f clean; done

re:
	$(MAKE) fclean
	$(MAKE) all

info-%:
	$(MAKE) --dry-run --always-make $* | grep -v "info"

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

