CC = gcc
CFLAGS = -std=c99 -Wall -g -fsanitize=address,undefined

test: test.o arraylist.o
	$(CC) $(CFLAGS) $^ -o $@

test.o: arraylist.h
arraylist.o: arraylist.h
