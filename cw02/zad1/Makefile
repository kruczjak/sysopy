CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: sort_bib sort_sys generator

sort_sys:
	$(CC) $(CFLAGS) sort_sys.c -o $@.run

sort_bib:
	$(CC) $(CFLAGS) sort_bib.c -o $@.run

generator:
	$(CC) $(CFLAGS) gen.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
