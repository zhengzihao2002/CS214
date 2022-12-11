#ifndef WW_H
#define WW_H
struct HarmonyOS;

struct Thread;
struct Thread * initializeThread();
void finalizeThreads(struct Thread ** array, int size);

struct DirectoryNode;
struct DirectoryNode * createDirectoryNode();
struct DirectoryQueue;
void d_insert(char * directory, struct DirectoryQueue * queue);
char * d_delete(struct DirectoryQueue *queue);

struct FileNode;
struct FileNode * createFileQueueNode();
struct FileQueue;
void f_insert(char * location, char * outputName,struct FileQueue * queue);
char ** f_delete(struct FileQueue *queue);

void initializeQueues();
void finalizeQueues();

void * dirWorker(void * arg);
void * filWorker(void * arg);
void recursiveWrap(char * directory,char** dir_array,char**fil_array,int dir_size,int fil_size);


void throwError(const char *path);
char * result;
void addTwoStrings(char * one ,char * str, char * two, char* result);
void store(char characters);
struct HarmonyOS * initialize(char *size);
void finalize(struct HarmonyOS  * localData);
void clear(char *array, int length);
int checkDirectory(const char * path);
int checkFile(const char * path);
int special_characters(char *ptr);
void print(char *array, int length, int mode, int type,struct HarmonyOS  * localData);
void print_word(int type,struct HarmonyOS  * localData);
void transfer(int mode, int type,struct HarmonyOS  * localData);
void empty_buffer(int mode, int type,struct HarmonyOS  * localData);
int isEmpty(char *array, int size);
char * insertWrap(char * path, int size);
void throwError(const char *path);
int checkTXT(char *name, int length);
int validness(struct dirent *file, char *directory_name, char *str);
void addTwoStrings(char *one, char *str, char *two, char *result);
void readFromDirectory(char *directory_name,struct HarmonyOS  * localData);
void readFromFile(char *path,struct HarmonyOS  * localData);
void readFromFileOutputToFile(char *path, char *output_path,struct HarmonyOS  * localData);
#endif