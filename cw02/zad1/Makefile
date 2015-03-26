CC = gcc
CFLAGS = -Wall -pedantic -std=c11

all: sort_bib sort_sys generator

sort_sys:
	$(CC) $(CFLAGS) sort_sys.c -o $@.run

sort_bib:
	$(CC) $(CFLAGS) sort_bib.c -o $@.run

generator:
	$(CC) $(CFLAGS) gen.c -o $@.run

test: all
	@echo "4bajty 100"
	./generator.run 100 4
	cp file.txt file2.txt
	./sort_sys.run file.txt 4
	./sort_bib.run file2.txt 4
	@echo "4bajty 1000"
	./generator.run 1000 4
	cp file.txt file2.txt
	./sort_sys.run file.txt 4
	./sort_bib.run file2.txt 4
	@echo "512bajtów 100"
	./generator.run 100 512
	cp file.txt file2.txt
	./sort_sys.run file.txt 512
	./sort_bib.run file2.txt 512
	@echo "512bajtów 1000"
	./generator.run 1000 512
	cp file.txt file2.txt
	./sort_sys.run file.txt 512
	./sort_bib.run file2.txt 512
	@echo "4096bajty 10"
	./generator.run 10 4096
	cp file.txt file2.txt
	./sort_sys.run file.txt 4096
	./sort_bib.run file2.txt 4096
	@echo "4096bajty 100"
	./generator.run 100 4096
	cp file.txt file2.txt
	./sort_sys.run file.txt 4096
	./sort_bib.run file2.txt 4096
	@echo "8192bajtów 10"
	./generator.run 10 8192
	cp file.txt file2.txt
	./sort_sys.run file.txt 8192
	./sort_bib.run file2.txt 8192
	@echo "8192bajtów 100"
	./generator.run 100 8192
	cp file.txt file2.txt
	./sort_sys.run file.txt 8192
	./sort_bib.run file2.txt 8192

.PHONY: clean
clean:
	rm -f *.o *~ *.a *.so *.run
