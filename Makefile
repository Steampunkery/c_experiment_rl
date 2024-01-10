IDIR=include
ODIR=build
LDIR=lib
CC=gcc
CFLAGS=-I$(IDIR) -I$(LDIR)/include -g3 $(shell pkg-config --cflags glib-2.0)

LIBS=-luncursed $(shell pkg-config --libs glib-2.0)

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

_LIBDEPS = flecs.h \
		   dijkstra.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS)) \
	   $(patsubst %,$(LDIR)/include/%,$(_LIBDEPS))

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
		  dijkstra.o

OBJ = $(patsubst %,$(ODIR)/%,$(_LIBOBJ)) \
	  $(patsubst %,$(ODIR)/%,$(_OBJ))

default: roguelike

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

roguelike: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

flecs: $(ODIR)/flecs.o
dijkstra: $(ODIR)/dijkstra.o

$(ODIR)/%.o: $(LDIR)/src/%.c $(LDIR)/include/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ roguelike
