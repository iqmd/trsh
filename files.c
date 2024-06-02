#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "globals.h"
#include "util.h"
#include <sys/stat.h>
#include <time.h>


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

void deleteFile(char* file){
    char *delPath = joinPath(trash_can, file);

    if(remove(delPath) == 0){
        printf("%s removed \n",file);
    }else{
        perror("Error Occurred");
    }
}

void deleteFromDB(file_stack *fs) {
    size_t i = 0;
    while (i < fs->count) {
        char* del_file = fs->deletedFile[i].file;
        int time_diff = (int)getTimeDiff(del_file);
        if (time_diff >= TIME_LIMIT) {
            deleteFile(del_file);
            memmove(&fs->deletedFile[i], &fs->deletedFile[i + 1],
                    (fs->count - i - 1) * sizeof(delFile));
            fs->count--;
        } else {
            i++;
        }
    }
}


