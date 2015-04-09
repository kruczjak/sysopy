CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11

all: file_counter

file_counter:
	$(CC) $(CFLAGS) file_counter.c -o $@

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so file_counter
