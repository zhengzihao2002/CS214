#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "rwl.h"

#define THREADS 4

#define READSLEEP 2
#define WRITESLEEP 3
#define QUIETSLEEP 1


rwlock_t rwl;


void *worker(void *arg)
{
	int i;
	int id = *(int *)arg;
	
	printf("%d: Starting\n", id);
	

	for (i = 0; i < 20; i++) {
		if ((i + id) % 5 == 0) {
			printf("%d: Write lock request\n", id);
			write_lock(&rwl);
			printf("%d: Write lock obtained\n", id);
			sleep(WRITESLEEP);
			printf("%d: Write lock release\n", id);			
			write_unlock(&rwl);
		} else {
			printf("%d: Read lock request\n", id);
			read_lock(&rwl);
			printf("%d: Read lock obtained\n", id);
			sleep(READSLEEP);
			printf("%d: Read lock release\n", id);
			read_unlock(&rwl);
		}
		
		sleep(QUIETSLEEP);
	}
	
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t tids[THREADS];
	int i, ids[THREADS];

	rwlock_init(&rwl);
	
	for (i = 0; i < THREADS; i++) {
		ids[i] = i;
		pthread_create(&tids[i], NULL, worker, &ids[i]);
	}
	
	for (i = 0; i < THREADS; i++) {
		pthread_join(tids[i], NULL);
	}
	
	rwlock_destroy(&rwl);
	
	return EXIT_SUCCESS;
}
