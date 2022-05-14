/*
File writing/directory reading demo

Compile:
    gcc rec5.c -o rec5
Run: One of
    ./rec5 DIRECTORY
    ./rec5 -posix FILE CONTENT...
    ./rec5 -c-stl FILE CONTENT...
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

/*
Write bytes from a null-terminated string into a file using the POSIX API.
*/
void write_file_posix(char *filename, char *content) {
    /*
    The flags in the second argument control:
     - How the file should be opened (read/write)
     - What happens if the file doesn't exist (error or create it)
     - What should be done with the file's contents if it doesn't exist (overwrite, append, or delete)
    
    The flags in the third argument control the permissions that will be applied to the file if it's
    created. These flags let the file's owner (i.e. the user who created the file) read and write to it.

    Try changing the flags and seeing how it affects the result, especially when writing multiple times.
    */
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("main failed to open file");
        return;
    }

    // Total number of bytes written to file so far
    size_t total_written = 0;
    // Number of bytes written with the latest write call
    ssize_t bytes_written;
    // Loop until we hit the end of the string or write returns an error
    while (content[total_written] != '\0' && (bytes_written = write(fd, content + total_written, 1)) != -1) {
        total_written += bytes_written;
    }
    if (bytes_written == -1) {
        perror("main failed to write bytes to file");
    } else {
        printf("Finished writing\n");
    }

    close(fd);
}

/*
Write bytes from a null-terminated string to a file using the C Standard Library API.
*/
void write_file_c_stl(char *filename, char *content) {
    /*
    "w" represents a collection of flags; try to figure out which ones by having the program
    write a few times.

    fopen doesn't provide a way to configure permissions; there is a function umask which will
    set the permissions for the next fopen call.
    */
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("main failed to open file");
        return;
    }

    // fwrite handles the loop logic required by the POSIX API
    if (fwrite(content, strlen(content), 1, file) == -1) {
        perror("main failed to write to file");
    } else {
        printf("Finished writing\n");
    }

    fclose(file);
}

/*
Print the contents of a directory, similar to the ls command
*/
void list_directory(char *directory_name) {
    DIR *directory = opendir(directory_name);
    if (directory == NULL) {
        perror("main failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG) {
            printf("%s (file)\n", entry->d_name);
        } else if (entry->d_type == DT_DIR) {
            printf("%s (directory)\n", entry->d_name);
        }
    }

    closedir(directory);
}

int main(int argc, char *argv[]) {
    // Read directory
    if (argc == 2) {
        char *directory_name = argv[1];
        list_directory(directory_name);
        return 0;
    }

    // Write file
    if (argc >= 4) {
        char *api = argv[1];
        // Switch the writing function based on the first argument
        void (*writer)(char *, char *);
        if (strcmp(api, "-posix") == 0) {
            writer = write_file_posix;
        } else if (strcmp(api, "-c-stl") == 0) {
            writer = write_file_c_stl;
        } else {
            printf("First argument (api) must be -posix or -c-stl\n");
            return 0;
        }

        char *filename = argv[2];

        for (int i = 3; i < argc; i++) {
            // Write each additional argument as a separate string
            char *content = argv[i];
            writer(filename, content);
        }
        
        return 0;
    }

    printf("Usage: One of\n\t./rec5 DIRECTORY\n\t./rec5 -posix FILE CONTENT...\n\t./rec5 -c-stl FILE CONTENT...\n");
    return 0;
}
