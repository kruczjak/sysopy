CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11 -lpthread

all: client server

client:
	$(CC) $(CFLAGS) $@.c -o $@.run

server:
	$(CC) $(CFLAGS) $@.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
