#ifndef HEX_PATCH_LIBRARY_H
#define HEX_PATCH_LIBRARY_H

#include <stdio.h>

#define BUFFER_SIZE     1e5

typedef enum {
    PAT_SUCCESS, PAT_NOTFOUND, PAT_FAILURE
} PAT_RESULT;

// starts with 0, support negative index, ends with -1
typedef struct {
    int left, right;
} range;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Search `search` bytes in `fp`, return an array of starting offsets
 * after matching `count` times. If `count` is 0, match all.
 *
 * @param  fp           file pointer of the file to be searched;
 * @param  count        stop searching after how many matches, set to 0 to match all;
 * @param  matched      will be set to length of the array returned;
 * @param  byte_len     length of `search`;
 * @param  search       byte array to be searched in `fp`;
 *
 * @return              pointer to file offsets array;
 */
long long *search_single(FILE *fp, const int count, int * matched,const size_t byte_len, const char *search);

/**
 * Search `search` bytes in `fp`, then replace all of the mathes in `range` with `replace`.
 * Note: `search` and `replace` should be the same length.
 *
 * @param  fp           file pointer of the file to be searched;
 * @param  match_range  range of matches to be replaced;
 * @param  byte_len     length of `search` and `replace`;
 * @param  search       byte array to be searched in `fp`;
 * @param  replace      byte array used to replace `search` in `fp`;
 *
 * @return              PAT_SUCCESS if succeeded, or other value if failed (PAT_NOTFOUND, PAT_FAILURE);
 */
PAT_RESULT patch_single(FILE *fp, const range match_range, const size_t byte_len, const char *search, const char *replace);

#ifdef __cplusplus
}
#endif

#endif // HEX_PATCH_LIBRARY_H
