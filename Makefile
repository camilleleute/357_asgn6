CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -pedantic -g

TARGET = batch
TESTPROG = testprog

all: batch testprog

batch: batch.o
	$(CC) $(CFLAGS) -o batch batch.o

testprog: testprog.o
	$(CC) $(CFLAGS) -o testprog testprog.o

batch.o: batch.c
	$(CC) $(CFLAGS) -c batch.c -o batch.o

testprog.o: testprog.c
	$(CC) $(CFLAGS) -c testprog.c -o testprog.o

debug: clean batch testprog

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes ./batch $(ARGS)

gdb: batch
	gdb ./batch

clean:
	rm -f batch batch.o testprog testprog.o

test: batch testprog
	./batch $(ARGS)
