typedef struct {
	int reading;
	int writers;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
	pthread_cond_t write_ready;
} rwlock_t;


void rwlock_init(rwlock_t *L);
void rwlock_destroy(rwlock_t *L);

void read_lock(rwlock_t *L);
void read_unlock(rwlock_t *L);
void write_lock(rwlock_t *L);
void write_unlock(rwlock_t *L);



