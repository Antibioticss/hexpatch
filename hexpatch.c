#include "hexpatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

long long *search_single(FILE *fp, const int count, int * matched,const size_t byte_len, const char *search) {
    // start building nxt
    int *nxt = (int *)malloc((byte_len + 1) * sizeof(int));
    nxt[0] = nxt[1] = 0;
    int tmp = 0;
    for (int i = 2; i <= byte_len; i++) {
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
    bool search_stop = false;
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (buffer == NULL) {
        perror("malloc \033[1;31merror\033[0m");
        free(nxt);
        return NULL;
    }
    int match_count = 0, match_len = count ? count + 1 : 100, cur_idx = 0;
    long long fp_offset = ftell(fp), fp_total, *match_idx = malloc(match_len * sizeof(long long));
    fseek(fp, 0, SEEK_END);
    fp_total = ftell(fp);
    fseek(fp, fp_offset, SEEK_SET);

    if (match_idx == NULL) {
        perror("malloc \033[1;31merror\033[0m");
        free(nxt);
        free(buffer);
        return NULL;
    }
    PAT_RESULT search_result = PAT_NOTFOUND;
    while (!search_stop) {
        size_t read_len;
        if (fp_total - fp_offset < BUFFER_SIZE) {
            read_len = sizeof(char) * fread(buffer, sizeof(char), BUFFER_SIZE, fp);
        }
        else {
            read_len = BUFFER_SIZE * fread(buffer, BUFFER_SIZE, sizeof(char), fp);
        }
        // printf("read_len: %lu\n", read_len);
        if (read_len != BUFFER_SIZE) {
            if (feof(fp)) {
                search_stop = true;
            }
            else if (ferror(fp)) {
                perror("fread \033[1;31merror\033[0m");
                search_result = PAT_FAILURE;
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
            if (cur_idx == byte_len) {
                search_result = PAT_SUCCESS;
                match_idx[match_count++] = fp_offset + i - byte_len + 1;
                cur_idx = nxt[cur_idx];
                if (match_count == match_len) {
                    // extend match_idx
                    match_len += 100;
                    long long *new_match = (long long*)realloc(match_idx, match_len * sizeof(long long));
                    if (new_match != NULL) {
                        match_idx = new_match;
                    }
                    else {
                        fprintf(stderr, "realloc \033[1;31merror\033[0m: out of memory!\n");
                        match_count = 0;
                        search_stop = true;
                        search_result = PAT_FAILURE;
                        break;
                    }
                }
                if (count && match_count == count) {
                    search_stop = true;
                    break;
                }
            }
        }
        fp_offset += read_len;
    }
    *matched = match_count;
    free(nxt);
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