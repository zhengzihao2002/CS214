#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include "ww.h"

#define BUFFER_SIZE 0 // same as max_length
void * BUFFERSIZE;
#define SUCCESS 0
#define FAILURE 1
#define NONE 2
int status = SUCCESS;
int close_ready;//if close ready is 1, it means that all threads have been created, and is ready to close all threads if specific conditions are met.This is a General sign that threads are all created.

//THE THREADS: 
//initially all threads are 1 under recursive (-r) mode. If recursive mode is NOT activated, we will use a SINGLE thread to do reading AND wrapping.
int threads_read=1;//number of threads used to read directory files
int threads_wrap=1;//number of threads to wrap regular files
typedef struct Thread {
    pthread_t tid;
}Thread;
int active_directory_threads;
int active_file_threads;
Thread * initializeThread(){
    Thread * t = malloc(sizeof(Thread));
    return t;
}
void finalizeThreads(Thread ** array, int size){
    for(int i=0;i<size;i++){
        free(array[i]);
    }
}
Thread ** readThreadsArray;
Thread ** wrapThreadsArray;



//THE DIRECTORY QUEUE:
typedef struct DirectoryNode{
    // A directory name
    char * directory;
    //the pointer to the next node
    struct DirectoryNode * next;
}DirectoryNode;
struct DirectoryNode * createDirectoryNode(){
    DirectoryNode *dirNode = malloc (sizeof(DirectoryNode));
    dirNode->directory = NULL;
    dirNode->next = NULL;
    return dirNode;
}
typedef struct DirectoryQueue{
    //the linkedlist head of the directory(s)
    DirectoryNode * head;
    // the lock that gives exclusive access to the queue
    pthread_mutex_t lock;

}DirectoryQueue;

void d_insert(char * directory, DirectoryQueue * queue){

    //get the empty node
    DirectoryNode * node =createDirectoryNode();
    //set the data into the queue
    node->directory=directory;
    //set the node to the current queue by attaching it to the head: because its a mess finding the tail
    pthread_mutex_lock(&queue->lock);
    node->next=queue->head;
    queue->head=node;
    pthread_mutex_unlock(&queue->lock);

}
char * d_delete(DirectoryQueue *queue){
    //we can assume that the queue always gives out the first node first therefore the one being deleted is always the head
    char * dir;
    //lock the queue
    pthread_mutex_lock(&queue->lock);
    //delete node 
    if(queue->head==NULL){
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }
    DirectoryNode * node = queue->head;
    dir= node->directory;
    if(queue->head->next==NULL){
        (queue->head)=NULL;
    }else{
        (queue->head)=(queue->head->next);
    }
    free(node);
    //unlock the queue 
    pthread_mutex_unlock(&queue->lock);


    return dir;
}

DirectoryQueue * DirQueue ;







//THE FILE QUEUE:
typedef struct FileNode{
    // A file number
    char * location;
    char * outputName;
    //the pointer to the next node
    struct FileNode * next;
}FileNode;
struct FileNode * createFileQueueNode(){
    FileNode *FilQueue = malloc (sizeof(FileNode));
    FilQueue->location=NULL;
    FilQueue->outputName=NULL;
    FilQueue->next = NULL;
    //pthread_mutex_init(&FilQueue->lock,NULL);
    return FilQueue;
}
typedef struct FileQueue{
    //the linkedlist head of the file nodes
    FileNode * head;
    // the lock that gives exclusive access to the queue
    pthread_mutex_t lock;
    //length
    int length;
}FileQueue;

void f_insert(char * location, char * outputName,FileQueue * queue){
    queue->length++;
    //get the empty node
    FileNode * node =createFileQueueNode();
    //set the data into the queue
    node->location=location;
    node->outputName=outputName;
    //set the node to the current queue by attaching it to the head: because its a mess finding the tail
    pthread_mutex_lock(&queue->lock);
    node->next=queue->head;
    queue->head=node;
    pthread_mutex_unlock(&queue->lock);

}
char ** f_delete(FileQueue *queue){
    //we can assume that the queue always gives out the first node first therefore the one being deleted is always the head
    char ** locations=malloc(2*sizeof(char*));
    //lock the queue
    pthread_mutex_lock(&queue->lock);
    //delete node
    if(queue->head==NULL){
        free(locations);
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }
    FileNode * node = queue->head;
    locations[0]=node->location;
    locations[1]=node->outputName;
    if(queue->head->next==NULL){
        (queue->head)=NULL;
    }else{
        (queue->head)=(queue->head->next);
    }
    free(node);
    queue->length--;
    //unlock the queue 
    pthread_mutex_unlock(&queue->lock);


    return locations;
}

FileQueue * FilQueue ;











void initializeQueues(){
    //start up for both directory queue and the file queue
    DirQueue = malloc (sizeof(DirectoryQueue)); DirQueue->head=NULL;
    FilQueue = malloc(sizeof(FileQueue)); FilQueue->head=NULL;FilQueue->length=0;
    pthread_mutex_init(&DirQueue->lock,NULL);
    pthread_mutex_init(&FilQueue->lock,NULL);
    return;
}
void finalizeQueues(){
    //Destroy all queues (locks and the queue itself (free) )
    pthread_mutex_destroy(&DirQueue->lock);
    pthread_mutex_destroy(&FilQueue->lock);
    free(DirQueue);
    free(FilQueue);
    return;
}






typedef struct HarmonyOS{
    // THE TEMPORARY ARRAY: STORES THE CURRENT WORD
    /**
     * @brief
     * temporary array to store a word
     */
    char *temp_array;
    /**
     * @brief
     * the length of the word that is currently being read.
     */
    int word_length;

    // THE BUFFER: WORDS ARE MOVED FROM TEMPORARY ARRAY TO BUFFER
    /**
     * @brief
     * "buffer" an array of characters where we store the written characters in a line
     * if the buffer is full, or cannot fit a word, we skip a line, then clear out
     * the buffer
     */
    char *buffer;

    /**
     * @brief
     * "max_length" an integer that specifies the maximum amount of characters a single line could fit,
     * also called the buffer length
     */
    int max_length;

    /**
     * @brief
     * Number of characters that currently lies in the buffer
     */
    int char_read;

    // THE STORAGE: WORDS THAT ARE READY TO ERECT INTO STANDARD OUTPUT
    /**
     * @brief
     * a place where characters go before uniformly erected into standard output
     */
    char *storage;
    int storage_index;

    /**
     * @brief
     * file_read is the file descriptor of the file being read, or input file
     */
    int file_read;
    /**
     * @brief
     * file_write is the file descriptor of the file being written to or output file
     */
    int file_write;
}HarmonyOS;


// The "Initializing/Finalizing" Functions
/**
 * @brief
 * this function is called at the beginning in the main function. It initializes the buffer using malloc
 * to request space on the heap, and gives values to other global variables
 */
HarmonyOS * initialize(char * size)
{
    HarmonyOS * machine = malloc(sizeof(HarmonyOS));
    // set max lengths, it is given
    machine->max_length = atoi(size);

    // use it to allocate memory in the heap
    machine->buffer = malloc(machine->max_length * sizeof(char));
    machine->temp_array = malloc(machine->max_length * sizeof(char));
    machine->storage = malloc(machine->max_length * sizeof(char));

    // initialize it to -1, indicating empty
    for (int i = 0; i < machine->max_length; i++)
    {
        machine->buffer[i] = -1;
        machine->temp_array[i] = -1;
        machine->storage[i] = -1;
    }

    // set the length of current word, current buffer length, and current storage length to 0
    machine->word_length = 0;
    machine->char_read = 0;
    machine->storage_index = 0;

    return machine;
}

/**
 * @brief
 * this function is called at the end of the program in the main function. This is called to free the buffer
 * from the heap, to prevent memory leaks.
 */
void finalize(HarmonyOS * machine)
{
    free(machine->buffer);
    free(machine->temp_array);
    free(machine->storage);

    free(machine);
}





// Checking-existence Functions
/**
 * @brief
 * Function to check whether a directory exists or not.
 * It returns 1 if given path is directory and  exists
 * otherwise returns 0.
 */
int checkDirectory(const char *path)
{
    if(path==NULL){
        return 0;
    }
    // a empty struct that will store information about a file/directory
    struct stat stats;

    // stat() function is used to list properties of a file identified by path.
    // It reads all file properties and dumps to stats structure.
    stat(path, &stats);

    // Check for file is a directory file
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}
int checkFile(const char *path)
{
    // a empty struct that will store information about a file/directory
    struct stat stats;

    /*
    Definition  :   stat() function is used to list properties of a file identified by path.
    What it does:   It reads all file properties and dumps to stats structure.
    stat() = 0  :   stat() was SUCCESSFUL. The information is returned in stats

    if the file, given the name by "path", is not in current directory, then stat will return -1 and also nothing will be stored in stats structure
    brief: stat() function tries to access the information of the file (path) from the current directory
    */

    stat(path, &stats);
    // Checks if the file is a regular file
    if (S_ISREG(stats.st_mode))
    {
        // st_mode is the file type, checking if it is regular
        return 1;
    }
    return 0;
}






int special_characters(char *ptr)
{
    if (*ptr == ' ')
    {
        return 0;
    }
    if (*ptr == '\n')
    {
        return 0;
    }
    if (*ptr == '\0')
    {
        return 0;
    }
    if (*ptr == '\t')
    {
        return 0;
    }
    return 1;
}
void clear(char *array, int length)
{
    for (int i = 0; i < length; i++)
    {
        array[i] = -1;
    }
}





void print(char *array, int length, int mode, int type,HarmonyOS * localData)
{
    /*
    mode 1: print with a new line at the end
    mode 0: print without a new line at the end

    how this is useful:
        if it is a words that are regular, when we empty out buffer we want a new line at the end of the sentence.
        however, if we are reading a super long word that is longer than max length, we need to empty out the buffer
        then and continue reading, which doesn't require a new line since it is not a sentence, it is still the same word
    */

    /*
    TYPE
    type 1: any print statements will be towards standard output
    type 0: any print statements will be towards a specific file
    type 3: any print statements will be temporarily stored in a array, then released all at once at the end
    */
    // this function prints out all the characters in the array
    if (length == 0)
    {
        // useless call. Discontinue
        return;
    }
    for (int i = 0; i < length; i++)
    {
        if (type == 1)
            write(STDOUT_FILENO,&array[i],1);
        if (type == 0)
            write(localData->file_write, &array[i], 1);
    }

    if (mode == 1)
    {
        if (type == 1)
            write(STDOUT_FILENO,"\n",1);
        if (type == 0)
            write(localData->file_write, "\n", 1);

    }
}
void print_word(int type,HarmonyOS * localData)
{
    /*
    TYPE
    type 1: any print statements will be towards standard output
    type 0: any print statements will be towards a specific file
    type 3: any print statements will be temporarily stored in a array, then released all at once at the end
    */
    int bytes = 0;
    int wordcount=0;
    while ((bytes = read(localData->file_read, &localData->temp_array[localData->word_length], 1)) > 0 && (special_characters(&localData->temp_array[localData->word_length])) != 0)
    {   
        wordcount++;
        // read each character and print it out until we reach a space
        // word_length++;
        if (type == 1)
            write(STDOUT_FILENO, &localData->temp_array[localData->word_length], 1);
        if (type == 0)
            write(localData->file_write, &localData->temp_array[localData->word_length], 1);

    }
    if(wordcount!=0){
        //something is printed
         status=FAILURE;
    }
    if (type == 1)
        write(STDOUT_FILENO, "\n", 1);
    if (type == 0)
        write(localData->file_write, "\n", 1);

}
void transfer(int mode, int type,HarmonyOS * localData)
{
    /*
    MODE
    mode 1: print with a new line at the end
    mode 0: print without a new line at the end

    how this is useful:
        if it is a words that are regular, when we empty out buffer we want a new line at the end of the sentence.
        however, if we are reading a super long word that is longer than max length, we need to empty out the buffer
        then and continue reading, which doesn't require a new line since it is not a sentence, it is still the same word

    Your might ask : Why does "transfer", a function that transfers a singe word (stored in temp_array) into the buffer, need mode,
    where it decides whether to put a '\n' and the end? Because, transfer calls the function "print", which requires mode.
    print is called either from: readFromFile() or Transfer()
    */

    /*
    TYPE
    type 1: any print statements will be towards standard output
    type 0: any print statements will be towards a specific file
    */

    // this function transfers the current word into the buffer
    if (localData->word_length == 0)
    {
        // useless call. Discontinue
        return;
    }
    // checks if it fits into the buffer:
    if (localData->char_read + localData->word_length <= localData->max_length)
    {
        for (int i = 0; i < localData->word_length; i++)
        {
            localData->buffer[localData->char_read] = localData->temp_array[i];
            localData->char_read++;
        }
        // every word that is being transfered from temp_array to buffer only consists of the word characters. therefore we need to add a space at the end
        // if the buffer is full, clear the buffer and go to next line, no need of a space. if it is not, add a space and continue doing whats next!
        if (localData->char_read == localData->max_length)
        {
            print(localData->buffer, localData->char_read, mode, type,localData);
            clear(localData->buffer, BUFFER_SIZE);
            localData->char_read = 0;
        }
        else
        {
            localData->buffer[localData->char_read] = ' ';
            localData->char_read++;
        }
    }
    else if (localData->word_length < localData->max_length && localData->char_read + localData->word_length > localData->max_length)
    {
        // the word could fit into a empty buffer, but cannot fit here because the buffer left over space is too small
        // print out the words in the buffer and empty the buffer array, clear char_read, then put the new word in
        print(localData->buffer, localData->char_read, mode, type,localData);
        clear(localData->buffer, BUFFER_SIZE);
        localData->char_read = 0;

        for (int i = 0; i < localData->word_length; i++)
        {
            localData->buffer[i] = localData->temp_array[i];
            localData->char_read++;
        }
        if (localData->char_read == localData->max_length)
        {
            print(localData->buffer, localData->char_read, mode, type,localData);
            clear(localData->buffer, BUFFER_SIZE);
            localData->char_read = 0;
        }
        else
        {
            localData->buffer[localData->char_read] = ' ';
            localData->char_read++;
        }
    }
    else if (localData->word_length == localData->max_length)
    {
        // we have this word that is currently same as max length, but could be longer since more characters are to be read
        print(localData->buffer, localData->char_read, mode, type,localData);
        clear(localData->buffer, BUFFER_SIZE);
        localData->char_read = 0;

        if (type == 1)
            write(STDOUT_FILENO, "\n", 1);
        if (type == 0)
            write(localData->file_write, "\n", 1);

        for (int i = 0; i < localData->word_length; i++)
        {
            localData->buffer[i] = localData->temp_array[i];
            localData->char_read++;
        }

        if (localData->char_read == localData->max_length)
        {
            print(localData->buffer, localData->char_read, mode, type,localData);
            clear(localData->buffer, BUFFER_SIZE);
            localData->char_read = 0;
        }
        else
        {
            localData->buffer[localData->char_read] = ' ';
            localData->char_read++;
        }
    }

    // clears the temp_array and sets the word_length to zero since the array is empty. These two lines are accessed everytime transfer() is called
    clear(localData->temp_array, localData->word_length);
    localData->word_length = 0;
}
void empty_buffer(int mode, int type,HarmonyOS * localData)
{
    // this function is used to empty out the buffer when reaching the end of the program
    /*
    MODE
    mode 1: print with a new line at the end
    mode 0: print WITHOUT a new line at the end

    how this is useful:
        if it is a words that are regular, when we empty out buffer we want a new line at the end of the sentence.
        however, if we are reading a super long word that is longer than max length, we need to empty out the buffer
        then and continue reading, which doesn't require a new line since it is not a sentence, it is still the same word
    */

    /*
    TYPE
    type 0: any print statements will be towards a specific file
    type 1: any print statements will be towards standard output
    type 3: any print statements will be temporarily stored in a array, then released all at once at the end
    */
    print(localData->buffer, localData->char_read, mode, type,localData);
    clear(localData->buffer, BUFFER_SIZE);
    localData->char_read = 0;
}
int isEmpty(char *array, int size)
{
    // this function checks whether a array is emtpy
    for (int i = 0; i < size; i++)
    {
        if (array[i] != -1)
        {
            // the array is not empty
            return 1;
        }
    }
    return 0;
}

/*
The following are "readFrom" functions. they are different but have mostly the same code
    1. readFromFile : Reading from given file and print to standard input
    2. readFromSTDIN : Reading from standard input and print to storage, then at last erect all storage characters to the standard output
    3. readFromOutputToFile : Reading from a given file 'A' and output to a given file 'B'
*/
void readFromFile(char *path,HarmonyOS * localData)
{

    // open the file then store the address in the file descriptor
    localData->file_read = open(path, O_RDONLY);
    if (localData->file_read == -1)
    {
        // if the file descriptor returns -1 it means open failed
        finalize(localData);
        throwError(path);
        exit(EXIT_FAILURE);
    }

    /**
     * @brief
     * number of bytes read. if bytes read equal zero, we reached the end of the file
     */
    int bytes = 0;
    /**
     * @brief
     * new line is the number of '\\n' we encounter. if we encounter two in a row, we start a new paragraph and set "skipped" to equal to 1
     */
    int newLine = 0; // number of '\n'. if we encounter 2, we start a new paragraph
    /**
     * @brief
     * skipped: represents whether we skipped a new line previously.
     * if skipped==1 that means we just did a '\\n' and if we are doing it again right after, we decline a double new line request.
     * if we reach anything like a space we set the value back to zero
     */
    int skipped = 0;
    while ((bytes = read(localData->file_read, &localData->temp_array[localData->word_length], 1)) > 0)
    {
        // read one byte at a time and store it in the buffer
        localData->word_length++;
        if (special_characters(&localData->temp_array[localData->word_length - 1]) == 0)
        {
            // temp_array only stores a single word. if we reach any special characters such as a SPACE, it marks the end of a word, so we transfer the word to the buffer
            if (localData->temp_array[localData->word_length - 1] == ' ')
            {
                // encountered a space: what we do is we remove that space and put the word into the buffer with (attempts) space at the end (if with space means > width, it wont do it and it will clear dump buffer after)
                localData->temp_array[localData->word_length - 1] = '\0';
                localData->word_length--;
                transfer(1, 1,localData);
                newLine = 0;
                skipped = 0;
                continue;
            }
            else if (localData->temp_array[localData->word_length - 1] == '\n')
            {
                localData->temp_array[localData->word_length - 1] = '\0';
                localData->word_length--;
                transfer(1, 1,localData);
                newLine++;
            }
            if (newLine == 2 && skipped == 0)
            {
                // encounters a double new line: start of a new paragraph
                // this only happens if we DID NOT start a new paragraph previously right before.
                newLine = 0;
                skipped = 1;
                transfer(1, 1,localData);
                empty_buffer(1, 1,localData);
                write(STDOUT_FILENO,"\n",1);
            }
        }
        else if (localData->word_length >= localData->max_length)
        {
            // this happens if we reach a super long word with no spaces in between: print out each character until we reach EOF or a space
            if(localData->word_length>localData->max_length) status = FAILURE;
            // transfer the word (not totally read) to the buffer, then empty out the buffer (without starting a new line at the end)
            transfer(0, 1,localData);
            empty_buffer(0, 1,localData);
            // print out the rest of the word
            print_word(1,localData);
            // Technically useless, but for purposes of edgy mistakes, transfer any words in the temp_array to buffer, then empty buffer
            transfer(1, 1,localData);
            empty_buffer(1, 1,localData);
        }
    }
    // to close out, transfer any words read to buffer, then empty out the buffer, then close the file
    transfer(1, 1,localData);
    empty_buffer(1, 1,localData);
    close(localData->file_read);
}

void readFromFileOutputToFile(char *path, char *output_path, HarmonyOS * localData)
{
    // open the file then store the address in the file descriptor
    if(path==NULL){
        localData->file_read=0;
        localData->file_write = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    }else{
        localData->file_read = open(path, O_RDONLY);
        localData->file_write = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    }
    if (localData->file_read == -1 || localData->file_write == -1)
        {
            // if the file descriptor returns -1 it means open failed
            finalize(localData);
            throwError(path);
            exit(EXIT_FAILURE);
        }
    

    /**
     * @brief
     * number of bytes read. if bytes read equal zero, we reached the end of the file
     */
    int bytes = 0;
    /**
     * @brief
     * new line is the number of '\\n' we encounter. if we encounter two in a row, we start a new paragraph and set "skipped" to equal to 1
     */
    int newLine = 0; // number of '\n'. if we encounter 2, we start a new paragraph
    /**
     * @brief
     * skipped: represents whether we skipped a new line previously.
     * if skipped==1 that means we just did a '\\n' and if we are doing it again right after, we decline a double new line request.
     * if we reach anything like a space we set the value back to zero
     */
    int skipped = 0;
    while ((bytes = read(localData->file_read, &localData->temp_array[localData->word_length], 1)) > 0)
    {
        // read one byte at a time and store it in the buffer
        localData->word_length++;
        if (special_characters(&localData->temp_array[localData->word_length - 1]) == 0)
        {
            // temp_array only stores a single word. if we reach any special characters such as a SPACE, it marks the end of a word, so we transfer the word to the buffer
            if (localData->temp_array[localData->word_length - 1] == ' ')
            {
                // encountered a space
                localData->temp_array[localData->word_length - 1] = '\0';
                localData->word_length--;
                transfer(1, 0,localData);
                newLine = 0;
                skipped = 0;
                continue;
            }
            else if (localData->temp_array[localData->word_length - 1] == '\n')
            {
                localData->temp_array[localData->word_length - 1] = '\0';
                localData->word_length--;
                transfer(1, 0,localData);
                newLine++;
            }
            if (newLine == 2 && skipped == 0)
            {
                // encounters a double new line: start of a new paragraph
                // this only happens if we DID NOT start a new paragraph previously right before.
                newLine = 0;
                skipped = 1;
                transfer(1, 0,localData);
                empty_buffer(1, 0,localData);
                write(localData->file_write, "\n", 1);
            }
        }
        else if (localData->word_length >= localData->max_length)
        {
            // this happens if we reach a super long word with no spaces in between: print out each character until we reach EOF or a space
            /**
             * This if statement is nessciary because if wordlength==maxlength, we still need it to go here
             * it will store a space, word terminator, in the array as well. if the wordlength==maxlength
             * error is going to occur because space+ wordlength>max length. Anyway, a maxlength word is 
             * going on a seperate line by itself any way. So if wordlength == maxlength, its still exits with success, unless
             * there was a flag to failure before
             */
            if(localData->word_length>localData->max_length) status = FAILURE;
            transfer(0, 0,localData);
            empty_buffer(0, 0,localData);

            print_word(0,localData);

            transfer(1, 0,localData);
            empty_buffer(1, 0,localData);

        }
    }
    transfer(1, 0,localData);
    empty_buffer(1, 0,localData);
    close(localData->file_read);

}
void readFromStandard(HarmonyOS * localData){
    write(STDOUT_FILENO,"Enter your text below\n",strlen("Enter your text below\n")+1);
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
    //reads from stdin and print into a file 
    readFromFileOutputToFile(NULL,"SystemData/data.txt",localData);
    //read from that file and print to stdout
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
    readFromFile("SystemData/data.txt",localData);
    //delete the file
    remove("SystemData/data.txt");
}
void readNonStandard(char * name, char ** argv){
    // Check file type of the third argument
    if (checkDirectory(name) != 0)
    {
        // the argument recieved is a directory: Get all the ".txt" files in the directory
        HarmonyOS * localData = initialize(argv[1]);
        readFromDirectory(name,localData); 
        finalize(localData);
    }
    else if (checkFile(name) != 0)
    {
        // the argument recieved is a regular file or ".txt" file.
        HarmonyOS * localData = initialize(argv[1]);

        readFromFile(name,localData);

        finalize(localData);
    }
    else
    {
        // Oh no! File/Directory cannot be found!
        throwError(name);
    }
}
void throwError(const char *path)
{
    // Throws an error to the standard output and exits the program
    perror(path);
    exit(EXIT_FAILURE);
}
int checkTXT(char *name, int length)
{
    // Checks if a given file name is a txt file by checking if it ends with ".txt"
    /* returns 0 if so, and 1 if not */
    if (name[length - 4] == '.' && name[length - 3] == 't' && name[length - 2] == 'x' && name[length - 1] == 't')
    {
        return 0;
    }
    return 1;
}
int checkWRAP(char *name, int length)
{
    if(name[0]=='w' && name[1]=='r' && name[2]=='a' && name[3]=='p'&& name[4]=='.'){
        return 1;
    }
    return 0;
}
int validness(struct dirent *file, char *directory_name, char *str)
{
    //FIRST OF ALL, CHECK IF IT IS A DIRECTORY
    int len = strlen(file->d_name) + strlen(directory_name) + 2;
    char *dir = malloc(len * sizeof(char));
    addTwoStrings(directory_name, str, file->d_name, dir);

    if (checkDirectory(file->d_name)&&strcmp(&file->d_name[0],".")!=0 &&strcmp(file->d_name,"..")!=0&&strcmp(file->d_name,".DS_Store")!=0&& checkTXT(file->d_name, strlen(file->d_name)) != 0 )
    {
        free(dir);
        return 6;
    }
    free(dir);
    // This function is a "helper" for readFromDirectory
    /*
        Check the file if:
            1. The length of the name is greater than 4 since ".txt" is 4 characters
            2. The file is a regular file that exist
            3. The file ends with a ".txt"
            4. If it does not start with "wrap."

        It will refuse to wrap the file if any one of these conditions are not met, on that file. that is , wrap.text.txt will be created on top of text.txt
        the fourth check will prevent something like this happening: "wrap.wrap.text.txt"
    */

    // First Check
    if (strlen(file->d_name) < 4)
    {
        return 1;
    }

    // Second Check
    int length = strlen(file->d_name) + strlen(directory_name) + 2;
    char *result = malloc(length * sizeof(char));
    addTwoStrings(directory_name, str, file->d_name, result);
    if (checkFile(result) == 0)
    {
        free(result);
        return 2;
    }
    free(result);

    // Third Check
    if (checkTXT(file->d_name, strlen(file->d_name)) != 0)
    {
        return 3;
    }

    //Fourth Check
    if(checkWRAP(file->d_name,strlen(file->d_name))){
        return 4;
    }
    
    return 0;
}
void addTwoStrings(char *one, char *str, char *two, char *result)
{
    // this function adds the second string to the first, with a '/' between them
    strcpy(result, one);
    // notice that if all three lines are strcat it givens a heap buffer overflow for no ducking reason. this is terrible. duck it
    strcat(result, str);
    strcat(result, two);
}
void readFromDirectory(char *directory_name,HarmonyOS * localData)
{
    // Given the name, it opens the directory
    DIR *directory_stream = opendir(directory_name);

    // Given the directory stream, it gets the next entry in the directory stream (initially the first entry)
    struct dirent *entry;
    //numbers of files sucessfully read
    int number_of_files=0;
    while ((entry = readdir(directory_stream)) != NULL)
    {
        // If we enter this loop, it means that "entry" have grabbed a valid file
        if (validness(entry, directory_name, "/") == 0)
        {
            /*
            Get full name to access the directory
            For example, we are in directory 'A' and the text is in 'directory_name' , say that is 'B'
            Therefore, to get text, we have to make the path: 'A/B' for read to access the files.
            "char * result" is the path
            */
            int length = strlen(entry->d_name) + strlen(directory_name) + 2;
            char *result = malloc(length * sizeof(char));
            addTwoStrings(directory_name, "/", entry->d_name, result);


            /*
            Get the new ".txt" name.
            For example, if the text file is called "HuoZhuoCaiYingWenTongYiTaiWanDao.txt", the new file where
            the output goes is "wrap.HuoZhuoCaiYingWenTongYiTaiWanDao.txt".
            */
            int length2 = strlen(entry->d_name) + strlen(directory_name) + 6 + 1;
            char *result2 = malloc(length2 * sizeof(char));
            //addTwoStrings("output/wrap", ".", entry->d_name, result2);
            addTwoStrings(directory_name,"/wrap.", entry->d_name, result2);


            readFromFileOutputToFile(result, result2,localData);

            free(result);
            free(result2);
            number_of_files++;
        }
        

    }
    closedir(directory_stream);
    if(number_of_files==0){
        if(status!=FAILURE){
            status=NONE;
        }
    }
}
int print_directory_queue(){
    int length=0;
    DirectoryNode * node=DirQueue->head;
    if(node==NULL){
        printf("Directory queue is empty\n");
    }
    while(node!=NULL){
        printf("%s\n",node->directory);
        node=node->next;
        length++;
    }
    return length;
}
void print_file_queue(){
    FileNode * node=FilQueue->head;
    if(node==NULL){
        printf("File queue is empty\n");
    }
    while(node!=NULL){
        printf("%s\n",node->location);
        node=node->next;
    }
    return;
}
void RecursiveReadFromDirectory(char *directory_name, DirectoryQueue * dq , FileQueue * fq)
{
    //given a directory name, we pass the subdirectories into the directory queue, and the regular text files into the file queue

    // Given the name, we opens the directory
    DIR *directory_stream = opendir(directory_name);
    // Given the directory stream, it gets the next entry in the directory stream (initially the first entry)
    struct dirent *entry;
    while ((entry = readdir(directory_stream)) != NULL)
    {
        // If we enter this loop, it means that "entry" have grabbed a regular text file
        if (validness(entry, directory_name, "/") == 0||((checkTXT(entry->d_name,strlen(entry->d_name))==0)&&checkWRAP(entry->d_name,strlen(entry->d_name))==0))
        {
            /*
            Get full name to access the file
            For example, we are in directory 'A' and the text is in 'directory_name' , say that is 'B'
            Therefore, to get text, we have to make the path: 'A/B' for read to access the files.
            "char * result" is the path
            */
            int length = strlen(entry->d_name) + strlen(directory_name) + 2;
            char *result = malloc(length * sizeof(char));
            addTwoStrings(directory_name, "/", entry->d_name, result);
            /*
            Get the new ".txt" name.
            For example, if the text file is called "HuoZhuoCaiYingWenTongYiTaiWanDao.txt", the new file where
            the output goes is "wrap.HuoZhuoCaiYingWenTongYiTaiWanDao.txt".
            */
            int length2 = strlen(entry->d_name) + strlen(directory_name) + 6 + 1;
            char *result2 = malloc(length2 * sizeof(char));
            addTwoStrings(directory_name,"/wrap.", entry->d_name, result2);
            //insert into the text file queue

            f_insert(result,result2,fq);
            
        }else if(validness(entry, directory_name, "/") == 6){
            
            //encountered a subdirectory
            int length3 = strlen(entry->d_name) + strlen(directory_name) + 2;
            char *result3 = malloc(length3 * sizeof(char));
            addTwoStrings(directory_name, "/", entry->d_name, result3);
            if(checkTXT(result3,length3-1)==0){
                free(result3);
            }else{
                //insert the subdirectory into the queue

                d_insert(result3,dq);
                
            }
        }

    }
    
    closedir(directory_stream);
}
int checkRecursive(char ** argv){
    //this function checks if the first two characters of the first argument given is "-r". returns 0 if so.
    int flag=1;
    if(argv[1][0]=='-' && argv[1][1]=='r'){
        flag=0;
    }

    return flag;
}
void getThreadNumber(char *arg){
    //this function sets the number of threads given
    //NOTICE: to compare two single characters, NEVER EVER use strcmp. that shit crazy. use ' == '

    int length=strlen(arg);
    if(length>100){
        write(STDOUT_FILENO,"Error: Too big of an argument\n",strlen("Error: Too big of an argument\n")+1);
    }else if(length==2){
        if(arg[0]=='-'&&arg[1]=='r'){
            threads_read=1;
            threads_wrap=1;
            return;
        }else{
            write(STDOUT_FILENO,"Error: check your argument\n",strlen("Error: check your argument\n")+1);
            exit(1);
        }
    }else if(length<2){
        write(STDOUT_FILENO,"Error: Too small of an argument\n",strlen("Error: Too small of an argument\n")+1);
        exit(1);
    }

    char * threadToReadDir=NULL; int TTRD=0;
    char * threadToWrapFil=NULL; int TTWF=0;
    int mode=1;
    for(int i=2;i<length;i++){

        if(arg[i]==','){
            mode=2;
            continue;
        }
        if(mode==1){
            threadToReadDir=realloc(threadToReadDir,TTRD+2);
            memcpy(&threadToReadDir[TTRD],&arg[i],2);
            TTRD++;
        }else{
            threadToWrapFil=realloc(threadToWrapFil,TTWF+2);
            memcpy(&threadToWrapFil[TTWF],&arg[i],2);
            TTWF++;
        }
    }
    threads_read=atoi(threadToReadDir);
    threads_wrap=atoi(threadToWrapFil);
    free(threadToReadDir);
    free(threadToWrapFil);

}
void exit_handler(void){
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
    if(status==NONE){
        write(STDOUT_FILENO,"Program Finishes NORMALLY with Status = |SUCCESS|\nNote that no files are read\n",strlen("Program Finishes NORMALLY with Status = |SUCCESS|\nNote that no files are read\n")+1);
    }else{
        write(STDOUT_FILENO,"Program Finishes NORMALLY with Status = |",strlen("Program Finishes NORMALLY with Status = |")+1);
        //    a?b:c -> if      'a'          then   'b'      otherwise     'c'
        write(STDOUT_FILENO,(status == FAILURE) ? "FAILURE" : "SUCCESS",strlen((status == FAILURE) ? "FAILURE" : "SUCCESS")+1);
        write(STDOUT_FILENO,"|\nNOTE: Failure means there is a word longer than ",strlen("|\nNOTE: Failure means there is a word longer than ")+1);
        write(STDOUT_FILENO,"maximum length",strlen("maximum length")+1);
        write(STDOUT_FILENO," characters, success means there isn't\n",strlen(" characters, success means there isn't\n")+1);
    }
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
}

void * dirWorker(void * arg){
    //this is the directroy worker. A directory thread will run this loop until every thing is done
    while(1){
        //this thread is going to constantly check the directory pool to see if there is anything left.
        //this thread can only terminate if there are no more directories/subdirectories left: queue is empty and all the directory reading threads are non-active

        if(DirQueue->head!=NULL){
            //we have something in the linkedlist : grab it and work on it
            char * directory_name = d_delete(DirQueue);
            if(directory_name==NULL){
                continue;
            }

            active_directory_threads++;
            RecursiveReadFromDirectory(directory_name,DirQueue,FilQueue);
            free(directory_name);
            active_directory_threads--;
        }else{
            sleep(1);
        }
        if(DirQueue->head==NULL && active_directory_threads==0 &&close_ready==1){
            break;
        }
    }

    return NULL;
}

void * filWorker(void * arg){
    //this is the file worker. A file thread will run this loop until every thing is done
    while(1){
        //this thread can only terminate if there are no more files left to wrap: queue is empty and all the file wrapping  + directory reading threads are non-active
        if(FilQueue->head!=NULL){
            
            char ** locations =f_delete(FilQueue);
            if(locations==NULL){
                //this happens when a thread A sees a head and attempts to grab it, but another thread B locks it already. 
                //when thread B finishes, thread A tries to delete a node that is NULL (thread B is working on last node)
                //therefore thread A will then try to delete a NULL node which would cause an error. 
                //if locations==NULL means that f_delete() caught that we are trying to delete a null node, therefore stop deleteing and return NULL (handles this correctly)
                continue;
            }

            active_file_threads++;
            char * textLocation = locations[0];
            char * outputLocation = locations[1];
           
            HarmonyOS * localData = initialize(BUFFERSIZE);
            
            readFromFileOutputToFile(textLocation,outputLocation,localData);
            finalize(localData);

            free(textLocation);
            free(outputLocation);
            free(locations);

            active_file_threads--;

        }else{
            sleep(1);
        }
        if(FilQueue->head==NULL &&active_directory_threads==0 && active_file_threads==0 && close_ready==1){
            break;
        }
        
    }
    
    return NULL;
}
void recursiveWrap(char * directory){

    /**
     * @brief 
     * Recrusive wrap means that user set up mode "-r", aside from main thread, we need to create:
     * "threads_wrap" amount of threads who handles wrapping the files grabbed from file queue
     * "threads_read" amount of threads who handles reading from directories from directory queue and, placing subdirectories back into directory queue and placing regular files into file queue
     * Note: without "-r" given by the user, we will only wrap within the main thread. 
     */

    //create all the queues
    initializeQueues();

    //use the main thread to put the first directory into the queue
    d_insert(directory,DirQueue);

    //create the array of pointers to threads
    readThreadsArray=malloc(threads_read*sizeof(Thread*));
    wrapThreadsArray=malloc(threads_wrap*sizeof(Thread*));

    //set the active threads to zero
    active_file_threads=0;
    active_directory_threads=0;
    //create the directory reading threads
    for(int i = 0;i<threads_read;i++){
        Thread * readThread = initializeThread();
        readThreadsArray[i]=readThread;
        pthread_create(&readThread->tid,NULL,dirWorker,NULL);
    }

    //create the file wrapping threads
    for(int i=0;i<threads_wrap;i++){
        Thread * wrapThread = initializeThread();
        wrapThreadsArray[i]=wrapThread;
        pthread_create(&wrapThread->tid,NULL,filWorker,NULL);
    }

    //set signal that all threads are created and are ready to close when specific conditions are met
    close_ready=1;

    
    while(DirQueue->head!=NULL || FilQueue->head!=NULL || close_ready!=1){
        sleep(0);
    }
    //wait for the directory reading threads
    for(int i = 0;i<threads_read;i++){
        Thread * readThread = readThreadsArray[i];
        pthread_join(readThread->tid,NULL); 
    }

    //wait for the file wrapping threads
    for(int i=0;i<threads_wrap;i++){
        Thread * wrapThread = wrapThreadsArray[i];
        pthread_join(wrapThread->tid,NULL); 
    }
    
    //finalize the all threads and the array (free)
    finalizeThreads(readThreadsArray,threads_read);
    finalizeThreads(wrapThreadsArray,threads_wrap);
    free(readThreadsArray);
    free(wrapThreadsArray);

    finalizeQueues();
    
}
int main(int argc, char **argv)
{

    if (argc < 2)
    {
        // We do not have enough arguments
        write(STDOUT_FILENO,"Check your inputs. Please give a maximum length, and if possible, the file or directory\n",strlen("Check your inputs. Please give a maximum length, and if possible, the file or directory\n")+1);
        exit(EXIT_FAILURE);
    }else if (argc == 2 ){
        // File name is not present, read from standard input using standard input
        #undef BUFFER_SIZE
        #define BUFFER_SIZE atoi(argv[1])
        if(BUFFER_SIZE==0){
            write(STDERR_FILENO,"Invalid Width\n",14);
            exit(EXIT_FAILURE);
        }
        HarmonyOS * localData = initialize(argv[1]);
        readFromStandard(localData);
        finalize(localData);
        // exit the program with either success or failure depending on whether a "failure flag" was set previously in the program
        atexit(exit_handler);
        if (status == FAILURE)
        {
            exit(EXIT_FAILURE);
        }
        else
        {
            exit(EXIT_SUCCESS);
        }
    }else if(argc==4 && checkRecursive(argv)==0){
        //recursive mode "-r" activated
        #undef BUFFER_SIZE
        #define BUFFER_SIZE atoi(argv[2])
        BUFFERSIZE=argv[2];
        if(BUFFER_SIZE==0){
            write(STDERR_FILENO,"Invalid Width\n",14);
            exit(EXIT_FAILURE);
        }
        getThreadNumber(argv[1]);
        char * directory = malloc((strlen(argv[3])+1)*sizeof(char));
        strcpy(directory,argv[3]);
        recursiveWrap(directory);
        atexit(exit_handler);
        if (status == FAILURE)
        {
            exit(EXIT_FAILURE);
        }
        else
        {
            exit(EXIT_SUCCESS);
        }
    }else if(argc>4 ||(argc==4 && checkRecursive(argv)==1) ){
        //recieved too many arguments or 4 arguments of unknown type
        write(STDOUT_FILENO,"Error: Too many Arguments\n",strlen("Error: Too many Arguments\n")+1);
        exit(EXIT_FAILURE);
    }
    

    #undef BUFFER_SIZE
    #define BUFFER_SIZE atoi(argv[1])
    if(BUFFER_SIZE==0){
        write(STDERR_FILENO,"Invalid Width\n",14);
        exit(EXIT_FAILURE);
    }
    //read From a Non standard input (From a single regular text file or a directory with multiple regular text files)
    char *name = argv[2]; // the name of the directory or .txt file
    readNonStandard(name, argv);

    // exit the program with either success or failure depending on whether a "failure flag" was set previously in the program
    atexit(exit_handler);
    if (status == FAILURE)
    {
        exit(EXIT_FAILURE);
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}
