#ifndef MYMALLOC_H
#define MYMALLOC_H

#define malloc(s) mymalloc(s, __FILE__, __LINE__)
#define free(p) myfree(p, __FILE__, __LINE__)



struct block{

	//metadata section
    char status;    //If the block is allocated. 0 - not. 1 - yes
	char size;      //The size of this block

    //client data / payload section
    char *payload;

    //pointer to the next block
    struct block *next; 
};

void *mymalloc(size_t size, char *file, int line );

int getSize(char *qian, char *bai, char *shi, char *ge);

void initialize();

void printMemory(int num);

void placeSize(int size, int metadata_index);

void setZero(int start, int end);

void error(int type,char*file,int line);

void myfree(void *p, char *file, int line);

void error(int type,char*file,int line);

void newMetadata(int num);

void coalesce();

void initialize();

void printMemory(int num);

struct block* initBlock();



#endif