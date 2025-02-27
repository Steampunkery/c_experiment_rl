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

LIBS        := m uncursed dijkstra rlsmenu flecs sockui
LIBS_TARGET := lib/uncursed/libuncursed.a lib/dijkstra/libdijkstra.a \
			   lib/rlsmenu/librlsmenu.a lib/flecs/libflecs.a lib/sockui/libsockui.a
# These .o files will be forced into the final executable. This is required for uncursed to work
LIB_FORCED_OBJS := $(addprefix lib/uncursed/src/,plugins/tty.o plugins/wrap_tty.o)

ROOT_DIR    := $(shell realpath .)
INCS        := include lib/uncursed/include lib/rlsmenu/ lib/flecs/include lib/dijkstra/include lib/sockui lib/STC/include
INCS        := $(INCS:%=$(ROOT_DIR)/%)

SRC_DIR     := $(shell realpath src)
SRCS        := ai.c component.c gui.c input.c item.c log.c main.c map.c \
			   monster.c observer.c player.c religion.c render.c systems.c \
			   socket_menu.c prefab.c arena.c ds.c random.c
SRCS        := $(SRCS:%=$(SRC_DIR)/%)

BUILD_DIR   := $(shell realpath build)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := gcc
CFLAGS		:= -g3 -Wall -Wextra -Werror -std=gnu11 -D_GNU_SOURCE
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
# info-%    print commands for dryrun %

all: $(NAME)

$(NAME): $(OBJS) $(LIBS_TARGET)
	$(CC) $(LDFLAGS) $(OBJS) $(LIB_FORCED_OBJS) $(LDLIBS) -o $(NAME)
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


uncursed: lib/uncursed/libuncursed.a

dijkstra: lib/dijkstra/libdijkstra.a

rlsmenu: lib/rlsmenu/librlsmenu.a

flecs: lib/flecs/libflecs.a

sockui: lib/sockui/libsockui.a

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re uncursed dijkstra rlsmenu flecs sockui
.SILENT:

