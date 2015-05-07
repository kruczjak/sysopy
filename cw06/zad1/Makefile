CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11

all: server client

server:
	$(CC) $(CFLAGS) $@.c -o $@.run
client:
	$(CC) $(CFLAGS) $@.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
