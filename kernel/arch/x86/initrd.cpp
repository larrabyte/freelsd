#include <mem/alloc.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <serial.hpp>
#include <initrd.hpp>
#include <vfs.hpp>

namespace initrd {
    root_header_t *rootheader;
    file_header_t *fileheaders;
    vfs::dirent_t dirent;
    vfs::node_t *rootnodes;
    vfs::node_t *root;
    vfs::node_t *devfs;
    int nodecount;

    uint32_t read(vfs::node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
        // Obtain the file header for our specific node.
        file_header_t header = fileheaders[node->inode];

        // Return zero if offset is bigger than file.
        // Otherwise, get size remaining until EOF.
        if(offset > header.filesize) return 0;
        if(offset + size > header.filesize) size = header.filesize - offset;

        // Start reading into buffer.
        memcpy(buffer, (uint8_t*) header.offset + offset, size);
        return size;
    }

    vfs::dirent_t *readdir(vfs::node_t *node, uint32_t index) {
        if(node == root && index == 0) {
            strcpy(dirent.name, "dev");
            dirent.ino = 0;
            return &dirent;
        }

        // Return a NULL pointer if index is too large.
        else if (index > (uint32_t) nodecount) return 0;

        strcpy(dirent.name, rootnodes[index - 1].name);
        dirent.ino = rootnodes[index - 1].inode;
        return &dirent;
    }

    vfs::node_t *finddir(vfs::node_t *node, const char *name) {
        if(node == root && strcmp(name, "dev") == 0) return devfs;

        for(int i = 0; i < nodecount; i++) {
            if(strcmp(name, rootnodes[i].name) == 0) return &rootnodes[i];
        }

        return 0;
    }

    vfs::node_t *initialise(uint32_t address) {
        // Retrieve the locations of the file table and the root header.
        fileheaders = (file_header_t*) (address + sizeof(root_header_t));
        rootheader = (root_header_t*) address;

        // Initialise the root directory.
        root = (vfs::node_t*) kmalloc(sizeof(vfs::node_t));
        memset(root, 0, sizeof(vfs::node_t));
        strcpy(root->name, "initrd");
        root->flags = VFS_DIRECTORY;
        root->finddir = &finddir;
        root->readdir = &readdir;
        root->read = &read;

        // Initialise the /dev directory.
        devfs = (vfs::node_t*) kmalloc(sizeof(vfs::node_t));
        memset(devfs, 0, sizeof(vfs::node_t));
        strcpy(devfs->name, "dev");
        devfs->flags = VFS_DIRECTORY;
        devfs->finddir = &finddir;
        devfs->readdir = &readdir;

        // Start allocating files from the ramdisk.
        rootnodes = (vfs::node_t*) kmalloc(sizeof(vfs::node_t) * rootheader->filecount);
        nodecount = rootheader->filecount;

        for(int i = 0; i < nodecount; i++) {
            // Adjust the offset to the beginning of the ramdisk IN memory.
            fileheaders[i].offset += address;

            // Create a new file node.
            memset(&rootnodes[i], 0, sizeof(vfs::node_t));
            strcpy(rootnodes[i].name, fileheaders[i].name);
            rootnodes[i].length = fileheaders[i].filesize;
            rootnodes[i].flags = VFS_FILE;
            rootnodes[i].read = &read;
            rootnodes[i].inode = i;
        }

        return root;
    }
}
