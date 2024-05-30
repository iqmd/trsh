#include <dirent.h>
#include <linux/limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include "files.h"
#include "util.h"
#include "globals.h"

#define TRASH_LOCATION ".trash"
#define CAPACITY 10
#define TRASH_DATABASE "trashdb"

const char* trash_can = NULL;
const char* home_dir = NULL;

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

file_stack fs;

void initDeletedFileArray(file_stack *fs){
    fs->deletedFile = (delFile *)malloc(sizeof(delFile)*CAPACITY);
    fs->count = 0;
    if(fs->deletedFile == NULL){
        perror("Failed to allocate memory for deletedFiles \n");
        exit(EXIT_FAILURE);
    }
    fs->capacity = CAPACITY;
    for(size_t i = fs->count; i < fs->capacity; i++){
        fs->deletedFile[i].file = NULL;
        fs->deletedFile[i].path = NULL;
    }
    printf("File stack initialized\n");
}

void addCapacity(file_stack *fs){
    size_t inc = fs->capacity*2;
    delFile *newArray = realloc(fs->deletedFile, sizeof(delFile)*inc);
    if(newArray == NULL){
        fprintf(stderr, "Failed to reallocate memory\n");
        exit(EXIT_FAILURE);
    }

    fs->deletedFile = newArray;
    fs->capacity = inc;
}

void pushDelFile(file_stack *fs, delFile df){
    if(fs->capacity == fs->count){
        addCapacity(fs);
    }
    fs->deletedFile[fs->count++] = df;
}

void initTrash(){
    if(mkdir(trash_can,0777) == -1){
        fprintf(stderr,"Trash is already initialized !!\n");
        exit(EXIT_FAILURE);
    }
    printf("Trash Initialized !!\n");
}


void updateLastTouch(char* filename){
    time_t now = time(NULL);
    struct utimbuf new_time;

    new_time.actime = now;
    new_time.modtime = now;

    if(utime(filename, &new_time) == -1){
        perror("An error occurred\n");
        exit(EXIT_FAILURE);
    }

}

void getCurrentPath(char* path){
    if((getcwd(path,PATH_MAX)) == NULL){
        perror("Error Occurred\n");
        exit(EXIT_FAILURE);
    }
}

void saveFileInfo(char* filename, char *path){
    delFile df;
    df.psize = strlen(path);
    df.fsize = strlen(filename);
    df.path = (char *) malloc(df.psize);
    df.file = (char *) malloc(df.fsize);
    strcpy(df.path,path);
    strcpy(df.file,filename);

    pushDelFile(&fs, df);
}

void readData(file_stack *fs){
    char *cache = joinPath(home_dir,".cache");
    char *db = joinPath(cache, TRASH_DATABASE);
    FILE *file = fopen(db, "rb");
    if(file != NULL){
        fread(&(fs->count), sizeof(size_t),1,file);
        fread(&(fs->capacity), sizeof(size_t),1,file);
        fs->deletedFile = (delFile *) malloc(sizeof(delFile)*fs->capacity);
        fread(fs->deletedFile, sizeof(delFile), fs->count, file);
        for(size_t i = 0; i < fs->count; i++){
            size_t *psize = &fs->deletedFile[i].psize;
            size_t *fsize = &fs->deletedFile[i].fsize;
            fread(psize, sizeof(size_t), 1, file);
            fread(fsize, sizeof(size_t), 1, file);
            fs->deletedFile[i].path = (char*) malloc(sizeof(char)* (*psize));
            fs->deletedFile[i].file = (char*) malloc(sizeof(char)* (*fsize));
            fread(fs->deletedFile[i].path, sizeof(char), *psize, file);
            fread(fs->deletedFile[i].file, sizeof(char), *fsize, file);
        }
        fclose(file);
    }else{
        printf("Initializing stack\n");
        initDeletedFileArray(fs);
    }
}

void writeData(file_stack *fs){
    char *cache = joinPath(home_dir,".cache");
    char *db = joinPath(cache, TRASH_DATABASE);
    FILE *file = fopen(db, "wb");
    if(file != NULL){
        fwrite(&(fs->count), sizeof(size_t), 1, file);
        fwrite(&(fs->capacity), sizeof(size_t), 1, file);
        fwrite(fs->deletedFile, sizeof(delFile), fs->count, file);
        for(size_t i = 0; i < fs->count; i++){
            fwrite(&(fs->deletedFile[i].psize), sizeof(size_t), 1, file);
            fwrite(&(fs->deletedFile[i].fsize), sizeof(size_t), 1, file);
            fwrite(fs->deletedFile[i].path, sizeof(char), fs->deletedFile[i].psize, file);
            fwrite(fs->deletedFile[i].file, sizeof(char), fs->deletedFile[i].fsize, file);
        }
        fclose(file);
    }else{
        perror("An error Occurred\n");
    }
}

int isDir(char *path){
    struct stat sb;
    if(stat(path,&sb) == 0){
        return S_ISDIR(sb.st_mode);
    }else{
        perror("Error Occurred");
        exit(EXIT_FAILURE);
    }
    return -1;
}

void helper(char* path);
void moveToTrash(char* filename, char* oldpath){
    char *src =  joinPath(oldpath, filename);
    char *dest = joinPath(trash_can,filename);

    if(isDir(src) == 1){
        printf("%s -> Directory \n",src);
        helper(src);
        return;
    }


    saveFileInfo(filename,oldpath);
    if(rename(src,dest) == 0){
        printf("File moved to : %s\n", dest);
        updateLastTouch(dest);
    }else{
        perror("Error Occured");
        exit(EXIT_FAILURE);
    }
}

void helper(char* path){
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(path);
    if(dp == NULL){
        perror("An error occurred");
        exit(EXIT_FAILURE);
    }

    while((dirp = readdir(dp)) != NULL ){
        char* filename = dirp->d_name;
        if(strcmp(filename,".") == 0 || strcmp(filename, "..") == 0){
            continue;
        }
        moveToTrash(filename,path);
    }

    if(rmdir(path) == -1){
        perror("Error Occurred");
    }
}

char* getHomeDir(){
    char *home = getenv("HOME");

    if(home == NULL){
        fprintf(stderr,"Error: Home variable not set\n");
    }

    return home;
}


int main(int argc, char** argv){

    char* filename;
    char path[PATH_MAX];
    /*TODO correct the home_dir thing we want to have a proper variable for
    home directory and not change it.*/
    home_dir = getHomeDir();
    trash_can = joinPath(home_dir, TRASH_LOCATION);

    readData(&fs);

    if (argc < 2) {
        fprintf(stderr, "trsh: missing operand\nTry 'trsh --help'");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], "--help") == 0){
        printf("Usage: trsh [OPTION] [FILE]\n  --init \t creates a .trash directory in the home directory\n  --clean \t remove files from .trash permanently\n");
        printf("%d", isDir(argv[2]));
        exit(EXIT_SUCCESS);
    }

    if(strcmp(argv[1], "--init") == 0){
        initTrash();
        exit(EXIT_SUCCESS);
    }

    if(strcmp(argv[1], "--clean") == 0){
        startDeleting();
        exit(EXIT_SUCCESS);
    }

    if(strcmp(argv[1], "--deleted") == 0){
        for(size_t i = 0; i < fs.count; i++){
            printf("%s \t %s\n", fs.deletedFile[i].file, fs.deletedFile[i].path);
        }
        exit(EXIT_SUCCESS);
    }


    if(access(trash_can, F_OK)){
        fprintf(stderr, "Trash not initialized\nUse:\t trsh --init");
        exit(EXIT_FAILURE);
    }




    for(int i = 1; i < argc; i++){
        filename = argv[i];
        getCurrentPath(path);
        updateLastTouch(filename);
        moveToTrash(filename, path);
        writeData(&fs);
    }

    return 0;
}
