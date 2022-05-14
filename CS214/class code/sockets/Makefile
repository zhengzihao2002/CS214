TARGETS = echos echos2 send
CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

all: $(TARGETS)

clean:
	rm -rf $(TARGETS) *.dSYM

echos echos2: %: %.c
	$(CC) $(CFLAGS) -pthread -o $@ $^

send: send.c
	$(CC) $(CFLAGS) -o $@ $^
	
