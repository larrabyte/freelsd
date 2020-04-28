#pragma once

#include <stdint.h>

static const char *root = "./tools/initrdgen/root/";  // Root directory of the initrd.
static const int maxfiles = 256;                      // Maximum number of files.

// A header to describe a file and it's location in the initrd.
typedef struct fileheader {
    uint32_t magic;
    uint32_t filesize;
    uint32_t offset;
    char name[32];
} file_header_t;

// Seen at the beginning of the initrd.
typedef struct fsheader {
    char magic[15];
    char builddate[11];
    uint32_t filecount;
} initrd_header_t;

// Get the size of a file given a filename.
uint32_t getfilesize(const char *filename);

// Basically just strcat(), but uses a big internal buffer.
char *combinestr(const char *s1, const char *s2);
