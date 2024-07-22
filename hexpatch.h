#ifndef HEX_PATCH_LIBRARY_H
#define HEX_PATCH_LIBRARY_H

#define PAT_SUCCESS     0
#define PAT_NOTFOUND    1
#define PAT_FAILURE     2

#define BUFFER_SIZE     1024

#include <stdio.h>

typedef struct {
    int left, right;
} range; // starts with 0, support negative index, ends with -1

int patch_single(FILE *fp, range match_range, const int count, const char *search, const char *replace);

#endif //HEX_PATCH_LIBRARY_H
