#include "hexpatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ASIZE           256
#define EXTEND_SIZE     100
#define BUFFER_SIZE     64 * 1024

typedef struct {
	int val, nxt;
} node;

long long *search_single(FILE *fp, const int count, int * matched, const size_t byte_len, const char *search) {
    /* preprocessing */
    int bufidx = 1, bucket[ASIZE];
	node *skipbuf = (node *)malloc((byte_len + 1) * sizeof(node));
    if (skipbuf == NULL) {
        perror("malloc \033[1;31merror\033[0m");
        return NULL;
    }
	memset(bucket, 0, sizeof(bucket));
	for (int i = 0; i < byte_len; i++) {
	    unsigned char ch = search[i];
		skipbuf[bufidx].val = i;
		skipbuf[bufidx].nxt = bucket[ch];
		bucket[ch] = bufidx++;
	}

    /* searching */
    bool search_stop = false;
    PAT_RESULT search_result = PAT_NOTFOUND;
    char *buffer = (char *)malloc((BUFFER_SIZE + byte_len * 2) * sizeof(char));
    /* add byte_len*2 because memcmp problem */
    if (buffer == NULL) {
        perror("malloc \033[1;31merror\033[0m");
        search_result = PAT_FAILURE;
        search_stop = true;
    }
    int match_count = 0, match_len = count ? count + 1 : EXTEND_SIZE;
    /* set to 100 and extend later if count is zero */
    long long fp_offset = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long long fp_total = ftell(fp);
    fseek(fp, fp_offset, SEEK_SET);
    long long *match_idx = malloc(match_len * sizeof(long long));
    if (match_idx == NULL) {
        perror("malloc \033[1;31merror\033[0m");
        search_result = PAT_FAILURE;
        goto search_exit;
    }
    while (!search_stop) {
        size_t read_len = BUFFER_SIZE + byte_len - 1, out_len;
        /* add byte_len-1 to prevent duplication */
        if (fp_total - fp_offset < read_len) {
            out_len = sizeof(char) * fread(buffer, sizeof(char), read_len, fp);
        }
        else {
            out_len = BUFFER_SIZE * fread(buffer, read_len, sizeof(char), fp);
        }
        if (out_len != read_len) {
            if (feof(fp)) {
                search_stop = true;
            }
            else if (ferror(fp)) {
                perror("fread \033[1;31merror\033[0m");
                search_result = PAT_FAILURE;
                goto search_exit;
            }
        }
        for (int i = byte_len - 1; i < read_len; i += byte_len) {
            for (int j = bucket[(unsigned char)buffer[i]]; j; j = skipbuf[j].nxt) {
                if (i - skipbuf[j].val <= read_len - byte_len) {
                    if (memcmp(search, buffer + i - skipbuf[j].val, byte_len) == 0) {
                        search_result = PAT_SUCCESS;
                        match_idx[match_count++] = fp_offset + i - skipbuf[j].val;
                        if (count && match_count == count) {
                            goto search_exit;
                        }
                        if (match_count == match_len) {
                            /* extend match_idx */
                            match_len += EXTEND_SIZE;
                            long long *new_match = (long long*)realloc(match_idx, match_len * sizeof(long long));
                            if (new_match != NULL) {
                                match_idx = new_match;
                            }
                            else {
                                fprintf(stderr, "realloc \033[1;31merror\033[0m: out of memory!\n");
                                match_count = 0;
                                search_result = PAT_FAILURE;
                                goto search_exit;
                            }
                        }
                    }
                }
            }
        }
        fp_offset += BUFFER_SIZE;
        /* move fp to fp_offset (otherwise fread will repeat) */
        if (fseek(fp, fp_offset, SEEK_SET) < 0) {
            perror("fseek \033[1;31merror\033[0m");
            goto search_exit;
        }
    }

search_exit:
    *matched = match_count;
    free(skipbuf);
    free(buffer);
    if (search_result == PAT_FAILURE) {
        free (match_idx);
        *matched = 0;
        match_idx = NULL;
    }
    return match_idx;
}

PAT_RESULT patch_single(FILE *fp, const range match_range, const size_t byte_len, const char *search, const char *replace) {
    if ((match_range.left >= 0 && match_range.right >= 0) || (match_range.left < 0 && match_range.right < 0)) {
        if (match_range.left > match_range.right) {
            fprintf(stderr, "range \033[1;31merror\033[0m: invalid range [%d, %d]\n", match_range.left, match_range.right);
            return PAT_FAILURE;
        }
    }

    int match_count = 0, matched;
    if (match_range.left >= 0 && match_range.right >= 0)
        match_count = match_range.right;
    long long *match_idx = search_single(fp, match_count, &matched, byte_len, search);
    if (match_idx == NULL) {
        return PAT_FAILURE;
    }
    if (matched == 0) {
        free(match_idx);
        return PAT_NOTFOUND;
    }

    PAT_RESULT patch_res = PAT_SUCCESS;
    int replace_start = match_range.left, replace_end = match_range.right;
    if (replace_start < 0) replace_start += matched;
    if (replace_end < 0) replace_end += matched;
    if (replace_start < 0 || replace_end < 0) {
        fprintf(stderr, "range \033[1;31merror\033[0m: negative index out of matched %d\n", matched);
        patch_res = PAT_FAILURE;
    }
    else if (replace_start > replace_end) {
        fprintf(stderr, "range \033[1;31merror\033[0m: invalid range after adding matched [%d, %d]\n", replace_start, replace_end);
        patch_res = PAT_FAILURE;
    }
    else {
        for (int i = replace_start; i <= replace_end; i++) {
            if (fseek(fp, match_idx[i], SEEK_SET) < 0) {
                perror("fseek \033[1;31merror\033[0m");
                patch_res = PAT_FAILURE;
                break;
            }
            const size_t written = fwrite(replace, byte_len, sizeof(char), fp);
            if (written != byte_len) {
                if (feof(fp)) {
                    fprintf(stderr, "fwrite \033[1;31merror\033[0m: unexpected eof!\n");
                    patch_res = PAT_FAILURE;
                    break;
                }
                if (ferror(fp)) {
                    perror("fwrite \033[1;31merror\033[0m");
                    patch_res = PAT_FAILURE;
                }
            }
        }
    }

    free(match_idx);

    return patch_res;
}