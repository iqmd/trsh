#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "util.h"
#include "globals.h"
#include "db.h"

#define CAPACITY 10
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

void saveFileInfo(char* filename, char *path, file_stack *fs){
    delFile df;
    df.psize = strlen(path);
    df.fsize = strlen(filename);
    df.path = (char *) malloc(df.psize);
    df.file = (char *) malloc(df.fsize);
    strcpy(df.path,path);
    strcpy(df.file,filename);

    pushDelFile(fs, df);
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
