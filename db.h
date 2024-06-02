#ifndef DB_H_
#define DB_H_
#include <sys/types.h>

#define TRASH_DATABASE "trashdb"
typedef struct {
    char *path;
    char *file;
    size_t psize;
    size_t fsize;
} delFile;


typedef struct {
    delFile *deletedFile;
    size_t count;
    size_t capacity;
} file_stack;

void initDeletedFileArray(file_stack *);
void addCapacity(file_stack *fs);
void pushDelFile(file_stack *fs, delFile df);
void saveFileInfo(char* filename, char *path, file_stack *fs);
void readData(file_stack *fs);
void writeData(file_stack *fs);


#endif // DB_H_
