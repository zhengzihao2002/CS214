#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "rwl.h"

#define DEBUG 1

#define str_(X) #X
#define str(X) str_(X)

#define strict(OP) \
	do { \
		int err = (OP); \
		if (err != 0) { \
			errno = err; \
			perror(__FILE__ ":" str(__LINE__) ":" #OP); \
			abort(); \
		} \
	} while(0)
	
#define lock(X) strict(pthread_mutex_lock(X))
#define unlock(X) strict(pthread_mutex_unlock(X))


void rwlock_init(rwlock_t *L)
{
	L->reading = 0;
	L->writers = 0;
	
	strict(pthread_mutex_init(&L->lock, NULL));
	strict(pthread_cond_init(&L->read_ready, NULL));
	strict(pthread_cond_init(&L->write_ready, NULL));
}

void rwlock_destroy(rwlock_t *L)
{
	strict(pthread_mutex_destroy(&L->lock));
	strict(pthread_cond_destroy(&L->read_ready));
	strict(pthread_cond_destroy(&L->write_ready));
}

void read_lock(rwlock_t *L)
{
	lock(&L->lock);
		if (DEBUG) printf("  > read_lock %d/%d\n", L->reading, L->writers);
		while (L->writers > 0) {
			if (DEBUG) puts("    read_lock waiting");
			strict(pthread_cond_wait(&L->read_ready, &L->lock));
		}
		
		++L->reading;
		if (DEBUG) printf("  < read_lock %d/%d\n", L->reading, L->writers);
	unlock(&L->lock);
}

void read_unlock(rwlock_t *L)
{
	lock(&L->lock);
		if (DEBUG) printf("  > read_unlock %d/%d\n", L->reading, L->writers);
		--L->reading;
		
		if (L->reading == 0) {
			strict(pthread_cond_signal(&L->write_ready));
		}
		if (DEBUG) printf("  < read_unlock %d/%d\n", L->reading, L->writers);
	unlock(&L->lock);
}

void write_lock(rwlock_t *L)
{
	lock(&L->lock);
		if (DEBUG) printf("  > write_lock %d/%d\n", L->reading, L->writers);
		++L->writers;
		while (L->reading != 0) {
			if (DEBUG) puts("    write_lock waiting");
			strict(pthread_cond_wait(&L->write_ready, &L->lock));
		}
		
		L->reading = -1;
		if (DEBUG) printf("  < write_lock %d/%d\n", L->reading, L->writers);
	unlock(&L->lock);
}

void write_unlock(rwlock_t *L)
{
	lock(&L->lock);
		if (DEBUG) printf("  > write_unlock %d/%d\n", L->reading, L->writers);
		--L->writers;
		L->reading = 0;
		
		if (L->writers == 0) {
			strict(pthread_cond_broadcast(&L->read_ready));
		} else {
			strict(pthread_cond_signal(&L->write_ready));
		}
		if (DEBUG) printf("  < write_unlock %d/%d\n", L->reading, L->writers);
	unlock(&L->lock);
}
