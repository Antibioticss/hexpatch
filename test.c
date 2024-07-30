#include "hexpatch.h"
#include <stdio.h>
#include <stdlib.h>

char data[] =
    "1. abc\n"
    "2. def\n"
    "3. abc\n"
    "4. def\n"
    "5. abc";
char buffer[100] = {0};

char data1[] = {0x11,0x6D,0x80,0x31,0x57,0x43,0xEB,0xDA};
// char data1[] = {0x16,0x54,0xA5,0x95};
char data2[] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};

int main() {
    // FILE * fp = fopen("test.txt", "w");
    // fwrite(data, sizeof data, 1, fp);
    // fclose(fp);
    //
    // fp = fopen("test.txt", "rb+");
    // PAT_RESULT pr = patch_single(fp, (range){0, -1}, 6, "5. abc", "fuckyu");
    // fclose(fp);
    //
    // fp = fopen("test.txt", "r");
    // fread(buffer, 1, 100, fp);
    // fclose(fp);
    // printf("buffer:\n%s\n", buffer);

    FILE *fp = fopen("random copy.bin", "rb+");
    PAT_RESULT pr = patch_single(fp, (range){0, -1}, 8, data1, data2);
    fclose(fp);

    printf("pr: %d\n", pr);

    // FILE *fp = fopen("random copy.bin", "rb+");
    // int matched;
    // long long *match_idx = search_single(fp, 1, &matched, 4, data1);
    // for (int i = 0; i < matched; i++) {
    //     printf("%llx,", match_idx[i]);
    // }
    // free(match_idx);
    return 0;
}