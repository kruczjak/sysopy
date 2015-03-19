CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: static_main shared_main dynamic_main

liblist.a:
	$(CC) $(CFLAGS) -c list.c -o list.o
	ar crs liblist_s.a list.o

liblist.so: list.c
	$(CC) $(CFLAGS) -fPIC -c list.c -o list.o
	$(CC) $(CFLAGS) -shared -fPIC -o liblist.so list.o

static_main: liblist.a
	$(CC) $(CFLAGS) main.c -o static_main -L. -llist_s

shared_main: liblist.so
	$(CC) -Wl,-rpath . $(CFLAGS) main.c -o shared_main -L. -llist

dynamic_main: liblist.so
	$(CC) -Wl,-rpath . $(CFLAGS) test2.c -o dynamic_main -ldl

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so static_main shared_main dynamic_main
