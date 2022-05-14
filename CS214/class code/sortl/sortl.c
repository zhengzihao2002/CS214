#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DEBUG 1

#define BUFSIZE 8
#define LISTLEN 16

char **lines;
int line_count, line_array_size;

void add_line(char *p)
{
    if (DEBUG) printf("Adding |%s|\n", p);
    if (line_count == line_array_size) {
	line_array_size *= 2;
	lines = realloc(lines, line_array_size * sizeof(char *));
	// TODO: check whether lines is NULL
    }

    lines[line_count] = p;
    line_count++;
}

int main(int argc, char **argv)
{
    int fd, bytes;
    char buf[BUFSIZE];
    char *crnt;
    int len;
    int pos, start;

    // TODO: move array list management to separate functions
    lines = malloc(sizeof(char *) * LISTLEN);
    if (!lines) {
	printf("malloc failed\n");
	return EXIT_FAILURE;
    }

    line_array_size = LISTLEN;
    line_count = 0;

    if (argc > 1) {
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
	    perror(argv[1]);
	    return EXIT_FAILURE;
	}
    } else {
	fd = 0;
    }

    crnt = NULL;
    len = 0;
    while ((bytes = read(fd, buf, 1)) > 0) {
	// read buffer and break file into lines
	start = 0;
	for (pos = 0; pos < bytes; pos++) {
			printf("%c\n",buf[pos]);

	    if (buf[pos] == '\n') {
		if (crnt == NULL) {
		    len = pos - start;
		    crnt = malloc(len + 1);
		    memcpy(crnt, &buf[start], len);
		} else {
		    len += pos;
		    crnt = realloc(crnt, len + 1);
		    memcpy(&crnt[len - pos], buf, pos);
		}
		crnt[len] = '\0';
		add_line(crnt);
		crnt = NULL;
		start = pos + 1;
	    }
	}

	if (start < pos) {
	    if (crnt == NULL) {
		len = pos - start;
		crnt = malloc(len + 1);
		memcpy(crnt, &buf[start], len);
	    } else {
		int newlen = len + (pos - start);
		crnt = realloc(crnt, newlen + 1);
		memcpy(&crnt[len], &buf[start], pos - start);
		len = newlen;
	    }
	    crnt[len] = '\0';  // technically unnecessary
	}
    }
    if (bytes == -1) {
	perror("read");
	return EXIT_FAILURE;
    }

    // if we reach here, we have read the entire file
    // sort and print the list
    

    return 0;
}
