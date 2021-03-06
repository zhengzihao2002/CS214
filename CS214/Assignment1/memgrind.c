#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "mymalloc.h"

/**
 * Tests at a glance:
 * 1. malloc() and immediately free() a 1-byte chunk, 120 times.
 * 2. Use malloc() to get 120 1-byte chunks, storing the pointers in an array, then use free() to deallocate the chunks.
 * 3. Chose randomly between two tasks until MALLOC has been called 120 times and free the remaining
 * 4. Two test of your own (see below)
 */
void test1(int num)
{
    // This test does NOT require the use of linked lists
    printf("Test 1 Running\n");
    for (int i = 1; i <= num; i++)
    {
        char *ptr = malloc(1);
        free(ptr);
    }
    printf("Test 1 Completed\n");
}
void test2(int num)
{
    // This test will require the implementation of array to store the pointers
    printf("Test 2 Running\n");
    // This is the array of size 120 we will store our pointers in
    char *array[num];
    for (int i = 0; i < num; i++)
    {
        // malloc 120 times and storing them in a pointer
        char *ptr = malloc(1);
        array[i] = ptr;
    }
    for (int i = 0; i < num; i++)
    {
        // getting the pointers from the array and freeing them
        free(array[i]);
    }
    // printMemory(1);
    printf("Test 2 Completed\n");
}
void test3(int num)
{
    printf("Test 3 Running\n");
    /*
    3. Randomly choose between
    • Allocating a 1-byte chunk and storing the pointer in an array
    • Deallocating one of the chunks in the array (if any)
    Repeat until you have called malloc() 120 times, then free all remaining allocated chunks.
    */
    int size = num * 2;
    char *array[size];
    int malloc_called = 0;
    int array_index1 = 0;
    int array_index2 = 0;
    int MAX = 1;
    int MIN = 0;
    srand(time(0));
    while (malloc_called < 120)
    {

        int num = rand() % (MAX + 1 - MIN) + MIN;
        /**
         * @brief Explanation of the random numbers
         * There could only be two possible numbers generated by the random generator:
         * (1) Allocate 1-byte chunk and store the pointer in a array
         * (2) Deallocate one of the chunks, if any
         */
        if (num == 1)
        {
            char *ptr = malloc(1);
            array[array_index1] = ptr;
            array_index1++;
            malloc_called++;
        }
        else if (num == 2)
        {
            if (array_index2 > array_index1)
            {
                continue;
            }
            free(array[array_index2]);
            array_index2++;
        }
    }
    //free all the remaining chunks
    while(array_index2<array_index1){
        free(array[array_index2]);
        array_index2++;
    }
    printf("Test 3 Completely\n");
}
void test4a(){
    //Test 4a) malloc 1-byte chunks until every single byte is full, then, free them from end to beginning. 
    // there could at most be 682 1-byte chunks: 682*(5+1)=4092 bytes
    char * array [682];
    for(int i=0;i<682;i++){
        char * ptr = malloc(1);
        array[i]=ptr;
    }
    for(int i=681;i>=0;i--){
        free(array[i]);
    }
}
void test4b()
{   
    //Test 4a) malloc 4090-byte chunk, then, free it, ten thousand times
    //there could at most be 4090 in an array because 4090 data size + 5 metadata size = 4095, which would be one byte left which cannot even fit another metadata
    for(int i=0;i<10000;i++){
        char * ptr=malloc(4090);
        free(ptr);
    }
}
int main(int argc, char **argv)
{   
    //Agreement with user
    printf("Hello TA! Welcome to My Malloc written by Alvin Zheng and XingChang Li!\n");
    printf("Please note that as this program approaches Test 4b, it may take around 44 seconds to complete.\nThis is completely normal, and please DO NOT THINK it is an infinite loop\n");
    printf("Again, please do not see this as an infinite loop! This is memory grinding!\n");
    printf("Enter 'AGREE' to agree\n");
    char agreement[20];
    scanf("%s",agreement);
    for(int i=0;i<20;i++){
        agreement[i]=tolower(agreement[i]);
    }
    int result=strcmp(agreement,"agree");
    if(result!=0){
        printf("Error in agreement. Please agree.\n");
        exit(EXIT_FAILURE);
    }







    // Performing Memory Grinding Tests: Required Tests
    test1(120);
    test2(120);
    test3(120);
    // Performing Memory Grinding Tests: Self Created Tests
    printf("Running Self Designed Tests\n");
    double sum_test4a_time=0;
    int rounds_a=0;
    for(int i=0;i<50;i++){
        clock_t t;
        t = clock();
        test4a();
        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC; // calculate the elapsed time
        sum_test4a_time+=time_taken;
        rounds_a++;
    }
    printf("Test 4a took an average of [%f] seconds to execute one time\n", (float)sum_test4a_time/rounds_a);

    double sum_test4b_time=0;
    int rounds_b=0;
    for(int i=0;i<50;i++){
        clock_t t;
        t = clock();
        test4b();
        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC; // calculate the elapsed time
        sum_test4b_time+=time_taken;
        rounds_b++;
    }
    printf("Test 4b took an average of [%f] seconds to execute one time\nTest 4b took a total of [%f] seconds to execute 50 times\n", (float)sum_test4b_time/rounds_b,(float)sum_test4b_time);
    printf("Self Designed Tests Completed\n");
    //printMemory(1);//prints memory in forms of ' metadata-clientSize ' where cleintSize is 1
}

