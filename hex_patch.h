#ifndef HEX_PATCH_LIBRARY_H
#define HEX_PATCH_LIBRARY_H

#define PAT_SUCCESS     0
#define PAT_NOTFOUND    1

#define BUFFER_SIZE     1024

#include <stdio.h>

int patch_single(FILE *fp, const int count, const char *search, const char *replace);

#endif //HEX_PATCH_LIBRARY_H
