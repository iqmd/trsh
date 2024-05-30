#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "util.h"
#include <sys/stat.h>
#include <time.h>

void deleteFile(char* file);

void getFileStat(struct stat* sb, char* name){
    if(stat(name, sb) == -1){
        perror("Error Occurred ");
        exit(1);
    }
}

double getTimeDiff(char* file){
    char *delPath = joinPath(trash_can, file);

    struct stat sb;
    getFileStat(&sb,delPath);
    time_t now  = time(NULL);
    double hours = difftime(now, sb.st_mtime)/3600;
    double days =  hours/24;

    return days;
}

void startDeleting(void){

    DIR *dp;
    struct dirent *dirp;
    dp = opendir(trash_can);
    if(dp == NULL){
        perror("An error occurred");
        exit(EXIT_FAILURE);
    }

    int files_deleted_count = 0;
    while((dirp = readdir(dp)) != NULL ){
        char* del_file = dirp->d_name;
        if(strcmp(del_file,".") == 0 || strcmp(del_file, "..") == 0){
            continue;
        }else{
            int time_diff = (int)getTimeDiff(del_file);
            if(time_diff >= TIME_LIMIT){
                deleteFile(del_file);
                files_deleted_count++;
            }
        }
    }
    printf("%d files removed", files_deleted_count);
    closedir(dp);
}


void deleteFile(char* file){
    char *delPath = joinPath(trash_can, file);

    if(remove(delPath) == 0){
        printf("%s removed \n",file);
    }else{
        perror("Error Occurred");
    }
}
