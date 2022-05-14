CC = gcc
CFLAGS = -std=c99 -Wall -g -fsanitize=address,undefined

memgrind: mymalloc.o memgrind.o
		$(CC) $(CFLAGS) $^ -o $@
%.0: %.c
		gcc -c -Wall $<
mymalloc.c: mymalloc.h
memgrind.c: mymalloc.h
