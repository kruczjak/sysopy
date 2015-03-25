CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: main

main:
	$(CC) $(CFLAGS) main.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
