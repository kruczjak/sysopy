CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11 -lpthread

all: 1 2 3 4 b

1:
	$(CC) $(CFLAGS) $@.c -o $@.run
2:
	$(CC) $(CFLAGS) $@.c -o $@.run
3:
	$(CC) $(CFLAGS) $@.c -o $@.run
4:
	$(CC) $(CFLAGS) $@.c -o $@.run
b:
	$(CC) $(CFLAGS) $@.c -o $@.run


.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
