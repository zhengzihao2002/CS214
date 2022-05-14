CC = gcc
CFLAGS = -g -Wall -fsanitize=address,undefined -std=c99

sortl: sortl.c 
	$(CC) $(CFLAGS) -o $@ $^

dsortl: sortl.c
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^

clean:
	rm -f sortl
