#include <stdio.h>
#include <stdlib.h>
#include "arraylist.h"

int main(int argc, char **argv)
{
    arraylist_t arr;
    al_init(&arr, 8, sizeof(int));

    int i, n;
    for (i = 0; i < 10; i++) {
	al_append(&arr, &i);
    }

    printf("There are %u elements\n", al_length(&arr));

    for (i = 0; i < al_length(&arr); i++) {
	al_index(&arr, &n, i);
	printf("Item %d: %d\n", i, n);
    }

    al_destroy(&arr);

    return EXIT_SUCCESS;
}
