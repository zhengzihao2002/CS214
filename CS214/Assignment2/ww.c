#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include "ww.h"

#define BUFFER_SIZE 0 // same as max_length
#define SUCCESS 0
#define FAILURE 1
int status = SUCCESS;

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
int storage_index=0;

// The "Initializing/Finalizing" Functions
/**
 * @brief
 * this function is called at the beginning in the main function. It initializes the buffer using malloc
 * to request space on the heap, and gives values to other global variables
 */
void initialize(char **argv)
{
    // set max lengths, it is given
    max_length = atoi(argv[1]);

    // use it to allocate memory in the heap
    buffer = malloc(max_length * sizeof(char));
    temp_array = malloc(max_length * sizeof(char));
    storage = malloc(max_length * sizeof(char));

    // initialize it to -1, indicating empty
    for (int i = 0; i < max_length; i++)
    {
        buffer[i] = -1;
        temp_array[i] = -1;
        storage[i] = -1;
    }

    // set the length of current word, current buffer length, and current storage length to 0
    word_length = 0;
    char_read = 0;
    storage_index = 0;
}
/**
 * @brief
 * this function is called at the end of the program in the main function. This is called to free the buffer
 * from the heap, to prevent memory leaks.
 */
void finalize()
{
    free(buffer);
    free(temp_array);
    free(storage);
}

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

// Checking-existence Functions
/**
 * @brief
 * Function to check whether a directory exists or not.
 * It returns 1 if given path is directory and  exists
 * otherwise returns 0.
 */
int checkDirectory(const char *path)
{
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
// TYPES AND MODE
#define noNewLine 520
#define yesNewLine 1314

#define toFile 5201314
#define standardOutput 3838438
#define toStorage 19491001

void print(char *array, int length, int mode, int type)
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
            write(file_write, &array[i], 1);
    }

    if (mode == 1)
    {
        if (type == 1)
            write(STDOUT_FILENO,"\n",1);
        if (type == 0)
            write(file_write, "\n", 1);

    }
}
void print_word(int type)
{
    /*
    TYPE
    type 1: any print statements will be towards standard output
    type 0: any print statements will be towards a specific file
    type 3: any print statements will be temporarily stored in a array, then released all at once at the end
    */
    int bytes = 0;
    int wordcount=0;
    while ((bytes = read(file_read, &temp_array[word_length], 1)) > 0 && (special_characters(&temp_array[word_length])) != 0)
    {   
        wordcount++;
        // read each character and print it out until we reach a space
        // word_length++;
        if (type == 1)
            write(STDOUT_FILENO, &temp_array[word_length], 1);
        if (type == 0)
            write(file_write, &temp_array[word_length], 1);

    }
    if(wordcount!=0){
        //something is printed
         status=FAILURE;
    }
    if (type == 1)
        write(STDOUT_FILENO, "\n", 1);
    if (type == 0)
        write(file_write, "\n", 1);

}
void transfer(int mode, int type)
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
    if (word_length == 0)
    {
        // useless call. Discontinue
        return;
    }
    // checks if it fits into the buffer:
    if (char_read + word_length <= max_length)
    {
        for (int i = 0; i < word_length; i++)
        {
            buffer[char_read] = temp_array[i];
            char_read++;
        }
        // every word that is being transfered from temp_array to buffer only consists of the word characters. therefore we need to add a space at the end
        // if the buffer is full, clear the buffer and go to next line, no need of a space. if it is not, add a space and continue doing whats next!
        if (char_read == max_length)
        {
            print(buffer, char_read, mode, type);
            clear(buffer, BUFFER_SIZE);
            char_read = 0;
        }
        else
        {
            buffer[char_read] = ' ';
            char_read++;
        }
    }
    else if (word_length < max_length && char_read + word_length > max_length)
    {
        // the word could fit into a empty buffer, but cannot fit here because the buffer left over space is too small
        // print out the words in the buffer and empty the buffer array, clear char_read, then put the new word in
        print(buffer, char_read, mode, type);
        clear(buffer, BUFFER_SIZE);
        char_read = 0;

        for (int i = 0; i < word_length; i++)
        {
            buffer[i] = temp_array[i];
            char_read++;
        }
        if (char_read == max_length)
        {
            print(buffer, char_read, mode, type);
            clear(buffer, BUFFER_SIZE);
            char_read = 0;
        }
        else
        {
            buffer[char_read] = ' ';
            char_read++;
        }
    }
    else if (word_length == max_length)
    {
        // we have this word that is currently same as max length, but could be longer since more characters are to be read
        print(buffer, char_read, mode, type);
        clear(buffer, BUFFER_SIZE);
        char_read = 0;

        if (type == 1)
            write(STDOUT_FILENO, "\n", 1);
        if (type == 0)
            write(file_write, "\n", 1);

        for (int i = 0; i < word_length; i++)
        {
            buffer[i] = temp_array[i];
            char_read++;
        }

        if (char_read == max_length)
        {
            print(buffer, char_read, mode, type);
            clear(buffer, BUFFER_SIZE);
            char_read = 0;
        }
        else
        {
            buffer[char_read] = ' ';
            char_read++;
        }
    }

    // clears the temp_array and sets the word_length to zero since the array is empty. These two lines are accessed everytime transfer() is called
    clear(temp_array, word_length);
    word_length = 0;
}

void empty_buffer(int mode, int type)
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
    print(buffer, char_read, mode, type);
    clear(buffer, BUFFER_SIZE);
    char_read = 0;
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
void readFromFile(char *path)
{

    // open the file then store the address in the file descriptor
    file_read = open(path, O_RDONLY);
    if (file_read == -1)
    {
        // if the file descriptor returns -1 it means open failed
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
    while ((bytes = read(file_read, &temp_array[word_length], 1)) > 0)
    {
        // read one byte at a time and store it in the buffer
        word_length++;
        if (special_characters(&temp_array[word_length - 1]) == 0)
        {
            // temp_array only stores a single word. if we reach any special characters such as a SPACE, it marks the end of a word, so we transfer the word to the buffer
            if (temp_array[word_length - 1] == ' ')
            {
                // encountered a space: what we do is we remove that space and put the word into the buffer with (attempts) space at the end (if with space means > width, it wont do it and it will clear dump buffer after)
                temp_array[word_length - 1] = '\0';
                word_length--;
                transfer(1, 1);
                newLine = 0;
                skipped = 0;
                continue;
            }
            else if (temp_array[word_length - 1] == '\n')
            {
                temp_array[word_length - 1] = '\0';
                word_length--;
                transfer(1, 1);
                newLine++;
            }
            if (newLine == 2 && skipped == 0)
            {
                // encounters a double new line: start of a new paragraph
                // this only happens if we DID NOT start a new paragraph previously right before.
                newLine = 0;
                skipped = 1;
                transfer(1, 1);
                empty_buffer(1, 1);
                write(STDOUT_FILENO,"\n",1);
            }
        }
        else if (word_length >= max_length)
        {
            // this happens if we reach a super long word with no spaces in between: print out each character until we reach EOF or a space
            if(word_length>max_length) status = FAILURE;
            // transfer the word (not totally read) to the buffer, then empty out the buffer (without starting a new line at the end)
            transfer(0, 1);
            empty_buffer(0, 1);
            // print out the rest of the word
            print_word(1);
            // Technically useless, but for purposes of edgy mistakes, transfer any words in the temp_array to buffer, then empty buffer
            transfer(1, 1);
            empty_buffer(1, 1);
        }
    }
    // to close out, transfer any words read to buffer, then empty out the buffer, then close the file
    transfer(1, 1);
    empty_buffer(1, 1);
    close(file_read);
}

void readFromFileOutputToFile(char *path, char *output_path)
{
    // open the file then store the address in the file descriptor
    if(path==NULL){
        file_read=0;
        file_write = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    }else{
        file_read = open(path, O_RDONLY);
        file_write = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    }
    if (file_read == -1 || file_write == -1)
        {
            // if the file descriptor returns -1 it means open failed
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
    while ((bytes = read(file_read, &temp_array[word_length], 1)) > 0)
    {
        // read one byte at a time and store it in the buffer
        word_length++;
        if (special_characters(&temp_array[word_length - 1]) == 0)
        {
            // temp_array only stores a single word. if we reach any special characters such as a SPACE, it marks the end of a word, so we transfer the word to the buffer
            if (temp_array[word_length - 1] == ' ')
            {
                // encountered a space
                temp_array[word_length - 1] = '\0';
                word_length--;
                transfer(1, 0);
                newLine = 0;
                skipped = 0;
                continue;
            }
            else if (temp_array[word_length - 1] == '\n')
            {
                temp_array[word_length - 1] = '\0';
                word_length--;
                transfer(1, 0);
                newLine++;
            }
            if (newLine == 2 && skipped == 0)
            {
                // encounters a double new line: start of a new paragraph
                // this only happens if we DID NOT start a new paragraph previously right before.
                newLine = 0;
                skipped = 1;
                transfer(1, 0);
                empty_buffer(1, 0);
                write(file_write, "\n", 1);
            }
        }
        else if (word_length >= max_length)
        {
            // this happens if we reach a super long word with no spaces in between: print out each character until we reach EOF or a space
            /**
             * This if statement is nessciary because if wordlength==maxlength, we still need it to go here
             * it will store a space, word terminator, in the array as well. if the wordlength==maxlength
             * error is going to occur because space+ wordlength>max length. Anyway, a maxlength word is 
             * going on a seperate line by itself any way. So if wordlength == maxlength, its still exits with success, unless
             * there was a flag to failure before
             */
            if(word_length>max_length) status = FAILURE;
            transfer(0, 0);
            empty_buffer(0, 0);

            print_word(0);

            transfer(1, 0);
            empty_buffer(1, 0);

        }
    }
    transfer(1, 0);
    empty_buffer(1, 0);
    close(file_read);

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
int validness(struct dirent *file, char *directory_name, char *str)
{
    // This function is a "helper" for readFromDirectory
    /*
        Check the file if:
            1. The length of the name is greater than 4 since ".txt" is 4 characters
            2. The file is a regular file that exist
            3. The file ends with a ".txt"
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
void readFromDirectory(char *directory_name)
{
    // Given the name, it opens the directory
    DIR *directory_stream = opendir(directory_name);

    // Given the directory stream, it gets the next entry in the directory stream (initially the first entry)
    struct dirent *entry;
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
            printf("Reading From: %s\n",result);
            /*
            Get the new ".txt" name.
            For example, if the text file is called "HuoZhuoCaiYingWenTongYiTaiWanDao.txt", the new file where
            the output goes is "wrap.HuoZhuoCaiYingWenTongYiTaiWanDao.txt".
            */
            int length2 = strlen(entry->d_name) + 4 + 2 + 7;
            char *result2 = malloc(length2 * sizeof(char));
            addTwoStrings("output/wrap", ".", entry->d_name, result2);

            printf("Outputting to: %s\n",result2);
            readFromFileOutputToFile(result, result2);

            free(result);
            free(result2);
        }
        // Uncomment Below to see what is valid files in a directory and what is not
        // printf("validness: %d           name: %s\n",validness(entry,directory_name),entry->d_name);
    }
    closedir(directory_stream);
}

int main(int argc, char **argv)
{
    
    if (argc < 2)
    {
        // We do not have enough arguments
        printf("Check your inputs. Please give a maximum length, and if possible, the file or directory\n");
        exit(EXIT_FAILURE);
    }
    else if (argc == 2)
    {
        // File name is not present, read from standard input using scanf
        #undef BUFFER_SIZE
        #define BUFFER_SIZE atoi(argv[1])
        if(BUFFER_SIZE==0){
            write(STDERR_FILENO,"Invalid Width\n",14);
            exit(EXIT_FAILURE);
        }
        initialize(argv);
        //reads from stdin and print into a file 
        readFromFileOutputToFile(NULL,"SystemData/data.txt");
        //read from that file and print to stdout
        write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
        readFromFile("SystemData/data.txt");
        //delete the file
        remove("SystemData/data.txt");
        finalize();
        // exit the program with either success or failure depending on whether a "failure flag" was set previously in the program
        //                        if      'a'          then   'b'      otherwise     'c'
        write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
        printf("Program Finishes NORMALLY with Status = |%s|\nNOTE: Failure means there is a word longer than %d characters, success means there isn't\n", (status == FAILURE) ? "FAILURE" : "SUCCESS",max_length);
        write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
        if (status == FAILURE)
        {
            exit(EXIT_FAILURE);
        }
        else
        {
            exit(EXIT_SUCCESS);
        }
    }


    #undef BUFFER_SIZE
    #define BUFFER_SIZE atoi(argv[1])
    if(BUFFER_SIZE==0){
        write(STDERR_FILENO,"Invalid Width\n",14);
        exit(EXIT_FAILURE);
    }
    char *name = argv[2]; // the name of the directory or .txt file
    // Check file type of the third argument
    if (checkDirectory(name) != 0)
    {
        // the argument recieved is a directory: Get all the ".txt" files in the directory
        initialize(argv);
        readFromDirectory(name);
        finalize();
    }
    else if (checkFile(name) != 0)
    {
        // the argument recieved is a regular file or ".txt" file.
        initialize(argv);

        readFromFile(name);

        finalize();
    }
    else
    {
        // Oh no! File/Directory cannot be found!
        throwError(name);
    }

    // exit the program with either success or failure depending on whether a "failure flag" was set previously in the program
    //                        if      'a'          then   'b'      otherwise     'c'
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
    printf("Program Finishes NORMALLY with Status = |%s|\nNOTE: Failure means there is a word longer than %d characters, success means there isn't\n", (status == FAILURE) ? "FAILURE" : "SUCCESS",max_length);
    write(STDOUT_FILENO,"---------------------------------------\n",strlen("---------------------------------------\n")+1);
    if (status == FAILURE)
    {
        exit(EXIT_FAILURE);
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}