#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

// REQUIREMENT: A must point to an allocated arraylist_t
// REQUIREMENT: capacity and elemsize are non-zero
// FIXME: al_init should fail if malloc() cannot allocate
void al_init(arraylist_t *A, index_t capacity, index_t elemsize)
{
    A->al_elemsize = elemsize;
    A->al_length = 0;
    A->al_capacity = capacity;

    A->al_data = malloc(capacity * elemsize);
}

// REQUIREMENT: A must point to an initialized arraylist_t with non-null data field
void al_destroy(arraylist_t *A)
{
    free(A->al_data);
}

int al_index(arraylist_t *A, void *dst, index_t index)
{
    if (index >= A->al_length) return 1;

    memcpy(dst, A->al_data + A->al_elemsize * index, A->al_elemsize);

    return 0;
}

int al_write(arraylist_t *A, index_t index, void *src)
{
    if (index >= A->al_length) return 0;

    memcpy(A->al_data + A->al_elemsize * index, src, A->al_elemsize);

    return 0;
}

index_t al_length(arraylist_t *A)
{
    return A->al_length;
}

void al_append(arraylist_t *A, void *src)
{
    if (DEBUG) printf("append [%u/%u]\n", A->al_length, A->al_capacity);
    if (A->al_length == A->al_capacity) {
	A->al_capacity *= 2;
	if (DEBUG) printf("append: increased capacity to %u\n", A->al_capacity);
	A->al_data = realloc(A->al_data, A->al_capacity * A->al_elemsize);
	// FIXME: should make sure realloc() didn't return NULL
    }

    memcpy(A->al_data + A->al_length * A->al_elemsize, src, A->al_elemsize);

    A->al_length++;
}

