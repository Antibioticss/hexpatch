#include "hexpatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int patch_single(FILE *fp, const int count, const char *search, const char *replace) {
    int patch_res = PAT_NOTFOUND;
    const long long orig_offset = ftell(fp);

    // start building nxt
    int *nxt = (int *)malloc((count + 1) * sizeof(int));
    nxt[0] = nxt[1] = 0;
    int tmp = 0;
    for (int i = 2; i <= count; i++)
    {
        while (tmp != 0 && search[tmp] != search[i-1]) {
            tmp = nxt[tmp];
        }
        if (tmp > 0 || search[tmp] == search[i-1]) {
            nxt[i] = ++tmp;
        }
        else {
            nxt[i] = 0;
        }
    }

    // start searching
    int match_count = 0, match_len = 100, cur_idx = 0;
    long long fp_offset = 0;
    long long *match_idx = (long long *)malloc(match_len * sizeof(long long));
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    bool search_end = false;
    while (!search_end) {
        const size_t read_len = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
        if (read_len != BUFFER_SIZE) {
            if (feof(fp)) {
                search_end = true;
            }
            else if (ferror(fp)) {
                perror("fread \033[1;31merror\033[0m");
                patch_res = PAT_FAILURE;
                break;
            }
        }
        for (int i = 0; i < read_len; i++) {
            while (cur_idx != 0 && search[cur_idx] != buffer[i]) {
                cur_idx = nxt[cur_idx];
            }
            if (cur_idx > 0 || search[cur_idx] == buffer[i]) {
                cur_idx++;
            }
            if (cur_idx == count) {
                patch_res = PAT_SUCCESS;
                match_idx[match_count++] = fp_offset + i - count + 1;
                cur_idx = nxt[cur_idx];
                if (match_count == match_len) {
                    match_len += 100;
                    long long *new_match = (long long *)realloc(match_idx, match_len * sizeof(long long));
                    if (new_match != NULL) {
                        match_idx = new_match;
                    } else {
                        fprintf(stderr, "realloc \033[1;31merror\033[0m: out of memory!\n");
                        match_count = 0;
                        search_end = true;
                        patch_res = PAT_FAILURE;
                        break;
                    }
                }
            }
        }
        fp_offset += read_len;
    }
    free(nxt);
    free(buffer);

    // start patching
    for (int i = 0; i < match_count; i++) {
        if (fseek(fp, orig_offset + match_idx[i], SEEK_SET) < 0) {
            perror("fseek \033[1;31merror\033[0m");
            patch_res = PAT_FAILURE;
            break;
        }
        const size_t written = fwrite(replace, count, sizeof(char), fp);
        if (written != count) {
            if (feof(fp)) {
                fprintf(stderr, "fwrite \033[1;31merror\033[0m: unexpected eof!\n");
                patch_res = PAT_FAILURE;
                break;
            }
            else if (ferror(fp)) {
                perror("fwrite \033[1;31merror\033[0m");
                patch_res = PAT_FAILURE;
            }
        }
    }
    free(match_idx);
    return patch_res;
}