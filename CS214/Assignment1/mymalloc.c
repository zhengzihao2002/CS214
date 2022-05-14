#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

#define MEMSIZE 4096
#define IN_USE 1
#define INITIALIZED 'I'
#define DATA 9
#define TRUE 666
#define FALSE 444
#define ZERO 0x00
//Below is what makes metadata a total of 5-bytes
#define METADATA_SIZE 4
#define METADATA_STATUS 1

static char memory[MEMSIZE];
/*

Here is a block of data:

|----||----||----||----||----| |----------------------------------------------|
|  A ||  B ||  C ||  D ||  E | |       F                                      |
|----||----||----||----||----| |----------------------------------------------|

METADATA: A , B , C , D , E
PAYLOAD/CLIENT DATA : F

metadata has a fixed size of 5 bytes : 1 byte for storing status, 4 byte for storing the size
*/

int getSize(char *qian, char *bai, char *shi, char *ge)
{
    int sum = 0;
    // get what is stored at qian
    sum += (*qian) * 1000;
    // get what is stored at bai
    sum += (*bai) * 100;
    // get what is stored at shi
    sum += (*shi) * 10;
    // get what is stored at ge
    sum += (*ge) * 1;
    return sum;
}
void initialize()
{
    // sets the memory array to a specific size and initialize to all zeros
    memset(memory, 0, MEMSIZE);

    // mark the array initialized
    memory[0] = INITIALIZED;

    memory[1] = (unsigned char)ZERO; // is the block in use?   '0'= NO     other = IN USE
    // size of payload in bytes
    memory[2] = (unsigned char)ZERO;
    memory[3] = (unsigned char)ZERO;
    memory[4] = (unsigned char)ZERO;
    memory[5] = (unsigned char)ZERO;
    return;
}
void printMemory(int num)
{
    int counter = 0;
    printf("%c\n", memory[0]);
    for (int i = 1; i < MEMSIZE; i++)
    {   
        if (counter ==  4 + num || counter==1)
        {
            printf(" ");
        }
        if (counter == 1 + 4 + num)
        {
            printf("\n");
            counter = 0;
        }

        printf("%d", memory[i]);
        counter++;
    }
    printf("\n");
}

void *mymalloc(size_t size, char *file, int line)
{
    // printf("Mymalloc called from %s:%d\n",file , line);
    // seaches for a empty space for malloc
    int alert = 0; // alert marks whether we should place a new metadata in after this one
    int index = 1; // index is ALWAYS the beginning of a metadata
    if (memory[0] != INITIALIZED)
    {
        // check whether it is initialized
        initialize();
    }
    if (size == 0)
    {
        // malloc 0, create new metadata, then return nothing
        return NULL;
    }
    while (index <= MEMSIZE)
    {
        // this loop will loop through every metablock

        if (index > MEMSIZE)
        {
            // we have reached the end of memory (4096) and have not found a space yet
            error(1,file,line);
            return NULL;
        }

        if (memory[index] == IN_USE)
        {
            // metadata shows that the following certain amount of bits are in use. skip this many bits to reach the next metadata
            int clientdata_size = getSize(&memory[index + 1], &memory[index + 2], &memory[index + 3], &memory[index + 4]);
            index = (index + 4 + clientdata_size + 1);
            if (index > MEMSIZE - 1)
            {
                error(1,file,line);
                return NULL;
            }
        }
        else if ((memory[index] == ZERO) && (memory[index + 1] != 0 || memory[index + 2] != 0 || memory[index + 3] != 0 || memory[index + 4] != 0))
        {
            // metadata shows that there is a specific amount of space
            int clientdata_size = getSize(&memory[index + 1], &memory[index + 2], &memory[index + 3], &memory[index + 4]);
            if ((size + 6) <= clientdata_size)
            {
                // fits: can fit our size and 5 byte new metadata, with at least one byte of free space associated to that new metadata
                if (index + 4 + size + 1 == IN_USE)
                {
                    // to the right to have a metadata block: set alert to not create a new metadata block
                    alert = 1;
                }
                break;
            }
            else
            {
                // doesn't fit, skip through
                index = (index + 4 + clientdata_size + 1);
                if (index > MEMSIZE - 1)
                {
                    error(1,file,line);
                    return NULL;
                }
            }
        }
        else
        {
            // metadata shows that there is NO PAYLOAD/CLIENTDATA and nothing after: place the new data in
            if ((index + 4 + size) > MEMSIZE - 1)
            {
                // check if the data can fit
                error(1,file,line);
                return NULL;
            }
            else if ((index + 4 + size >= MEMSIZE - 1))
            {
                alert = 1;
            }
            // the requested size fits : found the place, break out of the loop
            break;
        }
    }
    // at this point we finished the loop and found the correct place to place the data
    // mark the status to in use
    memory[index] = IN_USE;
    // set the size
    int num = size;
    int count = 4;
    while (num != 0)
    {
        int digit = num % 10;
        num = num / 10;
        memory[index + count] = digit;
        count--;
    }
    // create new metadata
    if(index+4+size+1+5>MEMSIZE-1){
        //if we dont have space to create the next metadata and at least one byte of free space
        alert=1;
    }
    if (alert == 0)
    {
        newMetadata(index + 4 + size + 1);
    }
    // return the pointer to the client data
    for (int i = 0; i < size; i++)
    {
        // temporarily store something in clientdata for testing purposes
        memory[index + 5 + i] = DATA;
    }
    return &memory[index + 5];
}

void myfree(void *p, char *file, int line)
{
    // printf("free called from %s:%d\n", file, line);
    char *client_data = (char *)p;

    // check some edgey cases
    if (memory[0] != INITIALIZED)
    {
        // check whether it is initialized
        error(2,file,line);
        return;
    }
    int known_address_flag = FALSE;
    for (int i = 0; i < MEMSIZE; i++)
    {
        if (client_data == &memory[i])
        {
            known_address_flag = TRUE;
            break;
        }
    }
    if (known_address_flag == FALSE)
    {
        // check whether we the pointer is a pointer to an valid memory address, in other words whether it is 'malloced'
        error(3,file,line);
        return;
    }
    if (*(client_data - 5) == 0)
    {
        // check for double free
        error(4,file,line);
        return;
    }
    if (*(client_data - 5) != 0 && *(client_data - 5) != 1)
    {
        // check for invalid metadata
        error(5,file,line);
        return;
    }
    // Check we can access the meta data
    char *first_digit, *second_digit, *third_digit, *fourth_digit;
    fourth_digit = (client_data - 1);
    third_digit = (client_data - 2);
    second_digit = (client_data - 3);
    first_digit = (client_data - 4);
    char *status = (client_data - 5);

    // free the cleintdata
    int size = ((*first_digit) * 1000) + ((*second_digit) * 100) + ((*third_digit) * 10) + ((*fourth_digit) * 1);
    // printf("Cleint Data Before:%d\n",*client_data);
    for (int i = 0; i < size; i++)
    {
        char *ptr = (char *)(p + i);
        *ptr = 0;
    }
    // printf("Client Data After:%d\n",*client_data);

    // change the status
    // printf("\nStatus Before:%d\n",*status);
    *status = 0;
    // printf("Status After:%d\n",*status);
    // see if we can coalesce the associated metadata with other empty metadatas
    coalesce();
    /*for(int i=0;i<MEMSIZE;i++){
        if(memory[i]!=0){
            printf("memory[%d]:%d\n",i,memory[i]);
        }
    }
    printf("All memory that is NOT '0' is printed\n");*/
}
void newMetadata(int num)
{
    int new_metadata_index = (num);        // index+4+size+1
    memory[new_metadata_index] = ZERO;     // status of new metadata
    memory[new_metadata_index + 1] = ZERO; // size of the block following the new metadata
    memory[new_metadata_index + 2] = ZERO;
    memory[new_metadata_index + 3] = ZERO;
    memory[new_metadata_index + 4] = ZERO;
}
void placeSize(int size, int metadata_index)
{
    // the purpose of this function is to place the given size to the specific index of the global memory array
    // set the size
    int num = size;
    int count = 4;
    while (num != 0)
    {
        int digit = num % 10;
        num = num / 10;
        memory[metadata_index + count] = digit;
        count--;
    }
}
void setZero(int start, int end)
{
    for (int i = start; i <= end; i++)
    {
        memory[i] = ZERO;
    }
}
void coalesce()
{   //First, check if all of the block is a free block, if so, coalese them into one block free block so it is fresh as just initialized
    int index=1;
    int flag=0;
    while(index<MEMSIZE-1){
        if(memory[index]==1){
            flag=1;
            break;
        }
        if(index+4>MEMSIZE-1)break;
        int s = getSize(&memory[index + 1], &memory[index + 2], &memory[index + 3], &memory[index + 4]);
        index=index+4+s+1;
    }
    if(flag==0){
        setZero(1,MEMSIZE-1);
    }
    //if not all are free, we coalesce certain blocks in specific areas
    int current_metadata_index = 1; // the index of the beginning of the metadata
    int previous_free_index = -1;   // the index of a metadata that is associated with a FREE block . Initially -1 indicating there isn't any previous FREE block
    int status = memory[current_metadata_index];
    int size = getSize(&memory[current_metadata_index + 1], &memory[current_metadata_index + 2], &memory[current_metadata_index + 3], &memory[current_metadata_index + 4]);
    int next_metadata_index = current_metadata_index + 4 + size + 1;

    while (current_metadata_index <= MEMSIZE - 2)
    {   
        status = memory[current_metadata_index];
        if(current_metadata_index+4>MEMSIZE-1){
            return;
        }
        size = getSize(&memory[current_metadata_index + 1], &memory[current_metadata_index + 2], &memory[current_metadata_index + 3], &memory[current_metadata_index + 4]);
        next_metadata_index = current_metadata_index + 4 + size + 1;
        if (status == 0)
        {
            /*
            status says that this metadata is associated with a free block:
            a) check if there is a previous free block, then coalesce if possible, then set this gigantic FREE block to previous
            b) if there isn't a free previous block, then set this as the first FREE block
            */
            if (previous_free_index == -1)
            {   
                previous_free_index = current_metadata_index;
                current_metadata_index = next_metadata_index;
                continue;
            }
            // Reaching this point: there is a previous free block
            int prev_size = getSize(&memory[previous_free_index + 1], &memory[previous_free_index + 2], &memory[previous_free_index + 3], &memory[previous_free_index + 4]);
            int sum_size= prev_size+size;
            placeSize(sum_size,previous_free_index);
            setZero(current_metadata_index,current_metadata_index+4+size);
            //coalesce success
            current_metadata_index=next_metadata_index;
            continue;
        }
        else
        {
            // status shows the client data associated with metadata is in use. skip this many bytes to reach the next.
            current_metadata_index = next_metadata_index;
            previous_free_index=-1; //we cant coalesce two blocks if there is a block in use between two free blocks
            continue;
        }
    }
}
void error(int type,char*file,int line)
{
    // When this function is called, it prints out the associated statement, then terminates the program
    switch (type)
    {
    case 1:
        printf("Program Teminated from %s %d: Error! Memory is Full!\n",file,line);
        exit(EXIT_FAILURE);
        break;
    case 2:
        printf("Program Teminated %s %d: Error! Attempting to free uninitialized array!\n",file,line);
        exit(EXIT_FAILURE);
        break;
    case 3:
        printf("Program Teminated %s %d: Error! Attempting to free a invalid address!\n",file,line);
        exit(EXIT_FAILURE);
        break;
    case 4:
        printf("Program Teminated %s %d: Error! Attempting to double free!\n",file,line);
        exit(EXIT_FAILURE);
    case 5:
        printf("Program Teminated %s %d: Error! Address passed to free MUST be pointer to the client data!\n",file,line);
        exit(EXIT_FAILURE);
    default:
        printf("Program Teminated %s %d: Error!\n",file,line);
        exit(EXIT_FAILURE);
        break;
    }
}
struct block *initBlock()
{
    // creates the first block
    struct block initial;

    initial.status = 0;
    initial.size = 0;

    struct block *ret = &initial;
    return ret;
}
