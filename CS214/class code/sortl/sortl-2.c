#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#define BUFSIZE 32
#define LISTLEN 16

char **lines;
int line_count, line_array_size;

void init_lines(void)
{
	lines = malloc(sizeof(char *) * LISTLEN);
	line_count = 0;
	line_array_size = LISTLEN;
}

void add_line(char *p)
{
	if (DEBUG)
		printf("Adding |%s|\n", p);
	if (line_count == line_array_size)
	{
		line_array_size *= 2;
		lines = realloc(lines, line_array_size * sizeof(char *));
		// TODO: check whether lines is NULL
	}

	lines[line_count] = p;
	line_count++;
}

int cmp_strings(const void *a, const void *b)
{
	return strcmp(*(char **)a, *(char **)b);
}

int main(int argc, char **argv)
{
	int fd, bytes;
	char buf[BUFSIZE];
	char *crnt;
	int len, crntlen;
	int pos, start;

	init_lines();

	if (argc > 1)
	{
		fd = open(argv[1], O_RDONLY);
		if (fd == -1)
		{
			perror(argv[1]);
			return EXIT_FAILURE;
		}
	}
	else
	{
		fd = 0;
	}

	crnt = NULL;
	crntlen = 0;
	while ((bytes = read(fd, buf, BUFSIZE)) > 0)
	{
		// read buffer and break file into lines
		if (DEBUG)
			printf("Read %d bytes\n", bytes);

		start = 0;
		for (pos = 0; pos < bytes; pos++)
		{
			if (buf[pos] == '\n')
			{
				if (DEBUG)
					printf("Segment %d:%d finished\n", start, pos);
				len = pos - start;
				crnt = realloc(crnt, len + crntlen + 1);
				memcpy(&crnt[crntlen], &buf[start], len);

				crntlen += len;

				crnt[crntlen] = '\0';
				add_line(crnt);

				crnt = NULL;
				crntlen = 0;
				start = pos + 1;
			}
		}

		// save any partial line at the end of the buffer
		if (start < pos)
		{
			if (DEBUG)
				printf("Segment %d:%d saved\n", start, pos);
			len = pos - start;
			crnt = realloc(crnt, len + crntlen + 1);
			memcpy(&crnt[crntlen], &buf[start], len);

			crntlen += len;
			crnt[crntlen] = '\0';
		}
	}
	if (bytes == -1)
	{
		perror("read");
		return EXIT_FAILURE;
	}

	// if we reach here, we have read the entire file
	if (crnt)
	{
		add_line(crnt);
	}

	// sort and print the list
	qsort(lines, line_count, sizeof(char *), cmp_strings);

	for (pos = 0; pos < line_count; pos++)
	{
		puts(lines[pos]);
	}

	return 0;
}
