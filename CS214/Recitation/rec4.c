/*
This file demonstrates how to read data from a file in a few ways.

Compile & run:
    gcc rec4.c -o rec4
    ./rec4 FILE

Compile & run, checking for memory leaks and open file descriptors:
    gcc rec4.c -o rec4 -g
    valgrind --leak-check=yes --track-fds=yes ./rec4 FILE

C's read API is pretty basic, but things get tricky when we try
to store the file's data in memory and work with it; since we
don't know in advance how many bytes we'll store, we need to
use dynamic memory (malloc/realloc/free).
*/

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
Print the content of a file.
 - fd is a file descriptor open for reading.
*/
void print_file(int fd) {
    /*
    Printing the file's content can be done with the following steps:
     (1) Read one byte from the file and save it
     (2) Print that byte
     (3) Repeat until we can't read any more bytes, either due to an error
         or because we reached the end of the file.
    */
    
    char c;  // Place to store the current byte
    ssize_t bytes_read;  // How many bytes we got from the last read
    /*
    This loop is equivalent to:

        while (true) {
            bytes_read = read(fd, &c, 1);
            if (bytes_read <= 0) {
                break;
            }
            printf("%c", c);
        }
    
    but it's a little more concise.
    */
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        printf("%c", c);
    }

    // read() returns -1 if an error occurred
    if (bytes_read < 0) {
        perror("main failed to read from file");
    }
}

/*
Load the entire content of a file into a string.
Since we don't know in advance how many bytes are in the file, the string
returned by this function is stored in dynamic memory, so make sure to free
it eventually!
 - fd is a file descriptor open for reading.
*/
char *read_content(int fd) {
    /*
    We can read the file byte-by-bytes as in the previous function.
    Another option is to read multiple bytes at a time (in "chunks"),
    reducing the number of times read() is called.

    IO functions (open/read/write/close/etc.) tend to be very slow
    relative to non-IO ones, so using them as few times as possible
    is often an important optimization in low-level programming.

    Try increasing chunk_size and adding some printf calls inside the
    while loop to see how many times read() is called as chunk_size
    increases.
    */
    size_t chunk_size = 1;

    char *content = malloc(chunk_size);  // Place to store the current chunk
    size_t content_length = 0;  // How many bytes we've read from the file so far
    ssize_t bytes_read;  // How many bytes we got from the last read
    // We want the next chunk to go into content AFTER the bytes we've already read;
    // content + content_length is a pointer to that location.
    while ((bytes_read = read(fd, content + content_length, chunk_size)) > 0) {
        content_length += bytes_read;
        // Make sure we have enough memory to store the next chunk
        content = realloc(content, content_length + chunk_size);
    }
    // Error checking
    if (bytes_read < 0) {
        perror("read_content failed to read from file");
        // content points to dynamic memory; since we're not returning it, free it
        free(content);
        return NULL;
    }

    // Add space for a null terminator
    content = realloc(content, content_length + 1);
    content[content_length] = '\0';

    return content;
}

/*
Read the next line from the file into a string.
 - fd is a file descriptor open for reading.
*/
char *read_line(int fd) {
    // The structure of this function is identical to the previous one,
    // but we use a fixed chunk size of 1.
    char *line = malloc(1);
    size_t line_length = 0;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, line + line_length, 1)) > 0) {
        line_length += 1;
        line = realloc(line, line_length + 1);
        // If the character we just read was a newline, stop reading.
        if (line[line_length - 1] == '\n') {
            break;
        }
    }
    // Error checking
    if (bytes_read < 0) {
        perror("read_line failed to read from file");
        free(line);
        return NULL;
    }
    // If line_length is 0, then we didn't read any bytes before the file
    // ended; this means there wasn't a line to read, so return NULL.
    if (line_length == 0) {
        free(line);
        return NULL;
    }

    line = realloc(line, line_length + 1);
    line[line_length] = '\0';

    return line;
}

/*
Read the content of the file as an array of lines, i.e. each line
is stored as a separate string. Returns the number of lines read.
 - fd is a file descriptor open for reading.
 - lines is a pointer to an array of strings; the lines will be
   placed inside this array.
*/
size_t read_lines(int fd, char ***lines) {
    *lines = malloc(0);  // Initialize the array as empty
    size_t lines_length = 0;  // Track how many lines we've collected

    char *line;  // Place to store the current line
    while ((line = read_line(fd)) != NULL) {
        // Expand the array pointed to by lines with space for another string
        *lines = realloc(*lines, (lines_length + 1) * sizeof(char *));
        // Store the line in the array
        (*lines)[lines_length] = line;
        // Record that we've stored another line
        lines_length += 1;
    }

    return lines_length;
}

int main(int argc, char *argv[]) {
    // Our program takes one argument, a filename
    if (argc < 2) {
        printf("Usage: ./rec4 FILE\n");
        return 0;
    }

    char *filename = argv[1];

    // Open the file in read-only mode
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("main failed to open file");
        return 0;
    }

    puts(" -- print_file -- ");
    print_file(fd);




    /*
    Rewind the file pointer back to the beginning of the file so
    the following code can read from it
    */
    lseek(fd, 0, SEEK_SET);

    puts(" -- read_content -- ");
    char *content = read_content(fd);
    if (content != NULL) {
        printf("%s", content);
        free(content);
    }

    lseek(fd, 0, SEEK_SET);

    puts(" -- read_line -- ");
    char *first_line = read_line(fd);
    if (first_line != NULL) {
        printf("%s", first_line);
        free(first_line);
    }

    lseek(fd, 0, SEEK_SET);

    puts(" -- read_lines -- ");
    char **lines;  // Array of strings
    size_t n_lines = read_lines(fd, &lines);
    for (int i = 0; i < n_lines; i++) {
        printf("%s", lines[i]);
        free(lines[i]);
    }
    free(lines);

    close(fd);

    return 0;
}
