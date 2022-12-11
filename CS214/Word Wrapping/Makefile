CC = gcc
CFLAGS = -g -Wall -fsanitize=address,undefined -std=c99

ww: ww.c 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f text/wrap.*
	rm -f text1/wrap.*
	rm -f text2/wrap.*
	rm -f text3/wrap.*