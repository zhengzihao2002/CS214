#ifndef WW_H
#define WW_H
void throwError(const char *path);
char * result;
void addTwoStrings(char * one ,char * str, char * two, char* result);
void store(char characters);
void initialize(char **argv);
void finalize();
int checkDirectory(const char * path);
int checkFile(const char * path);
int special_characters(char *ptr);
void print(char *array, int length, int mode, int type);
void print_word(int type);
void transfer(int mode, int type);
void empty_buffer(int mode, int type);
int isEmpty(char *array, int size);

void throwError(const char *path);
int checkTXT(char *name, int length);
int validness(struct dirent *file, char *directory_name, char *str);
void addTwoStrings(char *one, char *str, char *two, char *result);
void readFromDirectory(char *directory_name);
void readFromFile(char *path);
void readFromFileOutputToFile(char *path, char *output_path);
void readFromFile(char *path);
void readFromFileOutputToFile(char *path, char *output_path);
#endif