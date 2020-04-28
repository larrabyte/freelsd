#ifndef FREELSD_KERNEL_INITRD_HEADER
#define FREELSD_KERNEL_INITRD_HEADER

#include <stdint.h>
#include <vfs.hpp>

namespace initrd {
    typedef struct rootheader {
        char magic[15];
        char builddate[11];
        uint32_t filecount;
    } root_header_t;

    typedef struct fileheader {
        uint32_t magic;
        uint32_t filesize;
        uint32_t offset;
        char name[32];
    } file_header_t;

    extern root_header_t *rootheader;   // A pointer to the header of the initrd.
    extern file_header_t *fileheaders;  // A pointer to the header of the file table.
    extern vfs::node_t *rootnodes;      // A list of file nodes.
    extern vfs::node_t *root;           // The root directory node.
    extern vfs::node_t *devfs;          // A directory node for devfs.
    extern int nodecount;               // The number of nodes.

    // Read the contents of a node, given an offset, size and a buffer to write data to.
    uint32_t read(vfs::node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

    // Enumerate through the contents of a directory.
    vfs::dirent_t *readdir(vfs::node_t *node, uint32_t index);

    // Attempt to find a node with a given name, using node as the root.
    vfs::node_t *finddir(vfs::node_t *node, const char *name);

    // Initialise the initial ramdisk, given the module address.
    vfs::node_t *initialise(uint32_t address);
}

#endif
