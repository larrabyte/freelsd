#include "utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

static char combinedpath[4096];

char *combinestr(const char *s1, const char *s2) {
    sprintf(combinedpath, "%s%s", s1, s2);
    return combinedpath;
}

uint32_t getfilesize(const char *filename) {
    FILE *fstream = fopen(combinestr(root, filename), "r");

    if(fstream == NULL) {
        printf("[initrd] getfilesize(): cannot open %s at %s\n", filename, combinestr(root, filename));
        exit(-1);
    }

    fseek(fstream, 0, SEEK_END);
    uint32_t size = ftell(fstream);
    fclose(fstream);
    return size;
}
