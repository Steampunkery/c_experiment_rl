IDIR=include
ODIR=build
LDIR=lib
CC=gcc

ILIB=-I$(LDIR)/flecs/include
ILIB:=$(ILIB) -I$(LDIR)/DijkstraMap/include
ILIB:=$(ILIB) -I$(LDIR)/rlsmenu
CFLAGS=-I$(IDIR) $(ILIB) -g3 $(shell pkg-config --cflags glib-2.0) -Wall -Wextra -Werror

LFLAGS=-luncursed $(shell pkg-config --libs glib-2.0)

_DEPS = rogue.h \
		component.h \
		input.h \
		map.h \
		player.h \
		render.h \
		systems.h \
		ai.h \
		monster.h \
		gui.h \
		religion.h \
		log.h \
		observer.h \
		item.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS)) \
	   $(LDIR)/flecs/include/flecs.h \
	   $(LDIR)/DijkstraMap/include/dijkstra.h \
	   $(LDIR)/rlsmenu/rlsmenu.h

_OBJ =  main.o \
		component.o \
		map.o \
		player.o \
		render.o \
		systems.o \
		ai.o \
		monster.o \
		gui.o \
		input.o \
		religion.o \
		log.o \
		observer.o \
		item.o

_LIBOBJ = flecs.o \
		  dijkstra.o \
		  rlsmenu.o

OBJ = $(patsubst %,$(ODIR)/%,$(_LIBOBJ)) \
	  $(patsubst %,$(ODIR)/%,$(_OBJ))

default: roguelike

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

roguelike: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

flecs: $(ODIR)/flecs.o
dijkstra: $(ODIR)/dijkstra.o
rlsmenu: $(ODIR)/rlsmenu.o

$(ODIR)/flecs.o: $(LDIR)/flecs/src/flecs.c $(LDIR)/flecs/include/flecs.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/dijkstra.o: $(LDIR)/DijkstraMap/src/dijkstra.c $(LDIR)/DijkstraMap/include/dijkstra.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/rlsmenu.o: $(LDIR)/rlsmenu/rlsmenu.c $(LDIR)/rlsmenu/rlsmenu.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean flecs dijkstra rlsmenu default

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ roguelike
