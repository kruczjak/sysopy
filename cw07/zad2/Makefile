CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11 -lrt -lpthread -D_POSIX_C_SOURCE=200809L

all: main

main:
	$(CC) $(CFLAGS) $@.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
