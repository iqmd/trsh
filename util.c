#include "util.h"
#include <string.h>

void joinPath(char *left, char *right);

void joinPath(char* left,char* right){
    strcat(left, "/");
    strcat(left, right);
}
