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

const char* trash_can = NULL;


void initTrash(){
    if(mkdir(trash_can,0777) == -1){
        fprintf(stderr,"Trash is already initialized !!");
        exit(EXIT_FAILURE);
    }
    printf("Trash Initialized !!");
}


void updateLastTouch(char* filename){
    time_t now = time(NULL);
    struct utimbuf new_time;

    new_time.actime = now;
    new_time.modtime = now;

    if(utime(filename, &new_time) == -1){
        perror("An error occurred");
        exit(EXIT_FAILURE);
    }

}

void getCurrentPath(char* path){
    if((getcwd(path,PATH_MAX)) == NULL){
        perror("Error Occurred");
        exit(EXIT_FAILURE);
    }
}

void moveToTrash(char* filename, char* oldpath){
    joinPath(oldpath, filename);

    char trash[PATH_MAX];
    strcpy(trash,trash_can);
    joinPath(trash,filename);

    if(rename(oldpath,trash) == -1){
        perror("Error Occured");
        exit(EXIT_FAILURE);
    }
    updateLastTouch(trash);

    printf("File moved to : %s\n", trash);

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
    char *home_dir = getHomeDir();
    joinPath(home_dir,TRASH_LOCATION);
    trash_can = home_dir;

    if (argc < 2) {
        fprintf(stderr, "trsh: missing operand\nTry 'trsh --help'");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], "--help") == 0){
        printf("Usage: trsh [OPTION] [FILE]\n  --init \t creates a .trash directory in the home directory\n  --clean \t remove files from .trash permanently");
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


    if(access(trash_can, F_OK)){
        fprintf(stderr, "Trash not initialized\nUse:\t trsh --init");
        exit(EXIT_FAILURE);
    }


    for(int i = 1; i < argc; i++){
        filename = argv[i];
        getCurrentPath(path);
        updateLastTouch(filename);
        moveToTrash(filename, path);
    }

    return 0;
}
