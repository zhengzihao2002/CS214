#ifndef _ARRAYLIST_H_
#define _ARRAYLIST_H_

typedef unsigned int index_t;

typedef struct {
    char *al_data;
    index_t al_length;
    index_t al_capacity;
    index_t al_elemsize;
} arraylist_t;


void al_init(arraylist_t *a, index_t capacity, index_t elemsize);
void al_destroy(arraylist_t *a);
index_t al_length(arraylist_t *a);
void al_append(arraylist_t *a, void *item);
int al_pop(arraylist_t *a, void *dst);
int al_index(arraylist_t *a, void *dst, index_t index);
int al_write(arraylist_t *a, index_t index, void *src);


#endif
