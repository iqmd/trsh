#ifndef FILES_H_
#define FILES_H_
#include "db.h"

void startDeleting(void);
void getLastTouched(char* file);
void deleteFile(char*);
void deleteFromDB(file_stack *fs);


#endif // FILES_H_
