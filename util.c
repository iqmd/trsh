#include "util.h"
#include <string.h>
#include <stdlib.h>

char* joinPath(const char *left, const char *right);

char* joinPath(const char* left,const char* right){
    size_t left_size = strlen(left);
    size_t right_size = strlen(right);

    char *result = (char*) malloc(left_size+right_size+2);
    strcpy(result, left);
    strcat(result, "/");
    strcat(result, right);
    return result;
}
