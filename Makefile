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

debug: batch testprog


test: batch testprog
	./batch $(ARGS)
