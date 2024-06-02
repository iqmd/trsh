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
#include "db.h"

#define TRASH_LOCATION ".trash"

const char* trash_can = NULL;
const char* home_dir = NULL;
file_stack fs;


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


    saveFileInfo(filename,oldpath,&fs);
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
        deleteFromDB(&fs);
        writeData(&fs);
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
