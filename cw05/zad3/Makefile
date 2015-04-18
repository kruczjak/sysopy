CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11

all: main

main:
	$(CC) $(CFLAGS) $@.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
