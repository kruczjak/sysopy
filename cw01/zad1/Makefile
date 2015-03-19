CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: liblist.a liblist.so

liblist.a:
	$(CC) $(CFLAGS) -c list.c -o list.o
	ar crs liblist.a list.o

liblist.so: list.c
	$(CC) $(CFLAGS) -fPIC -c list.c -o list.o
	$(CC) $(CFLAGS) -shared -fPIC -o liblist.so list.o

.PHONY: clean
clean:
	rm -f *.o *~ core
