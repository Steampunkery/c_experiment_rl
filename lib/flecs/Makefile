CC=gcc
CFLAGS=-Wall -Werror -Wno-unused-function -Wextra -g3

default: libflecs.a

flecs.o: src/flecs.c include/flecs.h
	$(CC) -c -o $@ $< $(CFLAGS) -Iinclude

libflecs.a: flecs.o
	ar rcs $@ $^

.PHONY: clean

clean:
	rm -f *.o *.a
