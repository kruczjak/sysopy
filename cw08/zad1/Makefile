CC = gcc
CFLAGS = -Wall -pedantic -std=gnu11 -lpthread

all: main1 main2 main3 generator

main1:
	$(CC) $(CFLAGS) -DV1 main.c -o $@.run

main2:
	$(CC) $(CFLAGS) -DV2 main.c -o $@.run

main3:
	$(CC) $(CFLAGS) -DV3 main.c -o $@.run

generator:
	$(CC) $(CFLAGS) -lm $@.c -o $@.run

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run *.txt
