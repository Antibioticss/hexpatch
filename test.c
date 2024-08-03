#include "hexpatch.h"
#include <stdio.h>
#include <stdlib.h>

char data[] =
    "1. abc\n"
    "2. def\n"
    "3. abc\n"
    "4. def\n"
    "5. abc\n";
char buffer[100] = {0};

int main() {
    // FILE * fp = fopen("test.txt", "w");
    // fwrite(data, sizeof data, 1, fp);
    // fclose(fp);

    // fp = fopen("test.txt", "rb+");
    // PAT_RESULT pr = patch_single(fp, (range){0, -1}, 3, "abc", "xxx");
    // fclose(fp);

    // fp = fopen("test.txt", "r");
    // fread(buffer, 1, 100, fp);
    // fclose(fp);
    // printf("buffer:\n%s\n", buffer);

    char data1[] = {0x4F,0xB1,0x80,0x99,0xBA,0xEB,0xC9,0x00};
    char data2[] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
    FILE *fp = fopen("random copy.bin", "rb+");
    if (fp == NULL) exit(1);
    PAT_RESULT pr = patch_single(fp, (range){0, -1}, 8, data1, data2);
    fclose(fp);

    printf("pr: %d\n", pr);
    return 0;
}