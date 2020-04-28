#ifndef FREELSD_KERNEL_VFS_HEADER
#define FREELSD_KERNEL_VFS_HEADER

#include <stdint.h>

#define VFS_FILE         0x01
#define VFS_DIRECTORY    0x02
#define VFS_CHARDEVICE   0x03
#define VFS_BLOCKDEVICE  0x04
#define VFS_PIPE         0x05
#define VFS_SYMLINK      0x06
#define VFS_MOUNTPOINT   0x08

namespace vfs {
    // Read/write function pointers for filesystem drivers to implement.
    typedef uint32_t (*readfunc_t)(struct node*, uint32_t, uint32_t, uint8_t*);
    typedef uint32_t (*writefunc_t)(struct node*, uint32_t, uint32_t, uint8_t*);

    // Directory enumeration/searching function pointers for filesystem drivers to implement.
    typedef struct dirent* (*dir_readfunc_t)(struct node*, uint32_t);
    typedef struct node* (*dir_findfunc_t)(struct node*, const char*);

    // Open/close function pointers for filesystem drivers to implement.
    typedef void (*openfunc_t)(struct node*);
    typedef void (*closefunc_t)(struct node*);

    // Returned by readdir(), defined in POSIX specification.
    typedef struct dirent {
        char name[128];
        uint32_t ino;
    } dirent_t;

    typedef struct node {
        char name[128];
        uint32_t mask;
        uint32_t uid;
        uint32_t gid;
        uint32_t flags;
        uint32_t inode;
        uint32_t length;
        uint32_t magic;
        readfunc_t read;
        writefunc_t write;
        openfunc_t open;
        closefunc_t close;
        dir_readfunc_t readdir;
        dir_findfunc_t finddir;
        struct node *pointer;
    } node_t;

    // Standard read function (does not use file descriptors).
    uint32_t read(node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

    // Standard write function (does not use file descriptors).
    uint32_t write(node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

    // Standard open function (does not use file descriptors).
    void open(node_t *node, uint8_t read, uint8_t write);

    // Standard close function (does not use file descriptors).
    void close(node_t *node);

    // Iterate through a directory's contents.
    dirent_t *readdir(node_t *node, uint32_t index);

    // Find a child node, given a name.
    node_t *finddir(node_t *node, const char *name);
}

#endif
