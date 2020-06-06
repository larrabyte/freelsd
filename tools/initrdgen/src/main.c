#include "utils.h"

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(void) {
    // Headers for the initrd filesystem.
    file_header_t fileheads[256] = {0};
    initrd_header_t imghead;

    // Construct the initrd image header.
    long unixtime = time(NULL); struct tm *ts = localtime(&unixtime);
    snprintf(imghead.builddate, 11, "%02d/%02d/%04d", ts->tm_mday, ts->tm_mon + 1, ts->tm_year + 1900);
    strncpy(imghead.magic, "freelsd_initrd", 15);
    imghead.filecount = 0;

    // Open the root directory.
    DIR *rootdir = opendir(root);
    struct dirent *ent;
    if(rootdir == NULL) {
        printf("[ram] cannot open %s!\n", root);
        return -1;
    }

    // Create a new initrd image file.
    FILE *initrd = fopen("./isoroot/initrd.img", "wb");
    if(initrd == NULL) {
        printf("[ram] cannot create new initrd image!\n");
        return -1;
    }

    // Write the image header and the empty file headers.
    fwrite(&imghead, sizeof(initrd_header_t), 1, initrd);
    fwrite(&fileheads, sizeof(file_header_t), maxfiles, initrd);

    for(int i = 0; i < maxfiles; i++) {
        // Iterate through the next item.
        if((ent = readdir(rootdir)) != NULL) {
            // Skip it if it's a directory.
            if(ent->d_type == 4) {
                i--; continue;
            }

            // Find out the absolute path and filesize.
            char *fpath = combinestr(root, ent->d_name);
            size_t fsize = getfilesize(ent->d_name);

            // Open the file, error checking is always good :)
            FILE *fstream = fopen(fpath, "rb");
            if(fstream == NULL) {
                printf("[ram] main(): cannot open %s!\n", fpath);
                return -1;
            }

            // Write the header data for the file.
            strncpy(fileheads[i].name, ent->d_name, 32);
            fileheads[i].offset = ftell(initrd);
            fileheads[i].magic = 0xEFBEADDE;
            fileheads[i].filesize = fsize;

            // Read the file into memory and write.
            char *fdata = (char*) malloc(fsize);
            fread(fdata, fsize, 1, fstream);
            fwrite(fdata, fsize, 1, initrd);
            imghead.filecount++;

            // Cleanup so we don't consume everything.
            fclose(fstream);
            free(fdata);
        }
    }

    // Rewrite the headers to make them correct.
    fseek(initrd, 0, SEEK_SET);
    fwrite(&imghead, sizeof(initrd_header_t), 1, initrd);
    fwrite(&fileheads, sizeof(file_header_t), maxfiles, initrd);
    fclose(initrd);

    printf("[ram] Image generated with %d/%d files.\n", imghead.filecount, maxfiles);
    return 0;
}
