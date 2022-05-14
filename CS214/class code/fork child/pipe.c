#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fds[2];

    if (pipe(fds) == -1) {
	perror("pipe");
	abort();
    }

    int pid1 = fork();

    if (pid1 == 0) {
	close(fds[0]);
	if (dup2(fds[1], 1) == -1) {
	    perror("1 dup2");
	    abort();
	}

	execl("/bin/echo", "/bin/echo", "Hello!", NULL);

	perror("1 execl");
	abort();

	write(fds[1], "Hello!\n", 7);
	close(fds[1]);

	return EXIT_SUCCESS;
    
    } 

    printf("Echo is %d\n", pid1);

    int pid2 = fork();

    if (pid2 == 0) {
	close(fds[1]);
	if (dup2(fds[0], 0) == -1) {
	    perror("2 dup2");
	    abort();
	}

	execl("/bin/cat", "/bin/cat", "-", NULL);

	perror("2 execl");
	abort();

	puts("Fake cat is go!");

	int ch;
	while ((ch = getchar()) != EOF) {
	    putchar(ch);
	}

	puts("Exiting");
	return EXIT_SUCCESS;
    
    }

    close(fds[0]);
    close(fds[1]);

    printf("Cat is %d\n", pid2);

    int status;
    pid_t pid = wait(&status);

    printf("Child %d halted with %d\n", pid, WEXITSTATUS(status));

    pid = wait(&status);
    
    printf("Child %d halted with %d\n", pid, WEXITSTATUS(status));


    return EXIT_SUCCESS;
}
