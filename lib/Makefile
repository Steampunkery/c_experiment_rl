IDIR=include
SDIR=src
TDIR=test
CC=gcc
CFLAGS=-I$(IDIR) -g3 $(shell pkg-config --cflags glib-2.0)

LIBS=$(shell pkg-config --libs glib-2.0)

DIJKSTRA_DEPS=$(TDIR)/dijkstra.c $(SDIR)/dijkstra.c

default: dijkstra

dijkstra: $(DIJKSTRA_DEPS) $(IDIR)/dijkstra.h
	$(CC) -o $@ $(DIJKSTRA_DEPS) $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *~ core $(IDIR)/*~ dijkstra *.o
