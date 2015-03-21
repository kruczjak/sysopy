CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: generator

main:
	$(CC) $(CFLAGS) main.c -o $@.run

generator:
	$(CC) $(CFLAGS) gen.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
