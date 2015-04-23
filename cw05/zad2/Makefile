CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11

all: rdfifo wrfifo

wrfifo:
	$(CC) $(CFLAGS) $@.c -o $@
rdfifo:
	$(CC) $(CFLAGS) $@.c -o $@

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run rdfifo wrfifo
