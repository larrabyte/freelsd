#include <vfs.hpp>

namespace vfs {
    uint32_t read(node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
        if(node->read != 0) return node->read(node, offset, size, buffer);
        return 0;
    }

    uint32_t write(node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
        if(node->write != 0) return node->write(node, offset, size, buffer);
        return 0;
    }

    void open(node_t *node, uint8_t read, uint8_t write) {
        if(node->open != 0) node->open(node);
    }

    void close(node_t *node) {
        if(node->close != 0) node->close(node);
    }

    dirent_t *readdir(node_t *node, uint32_t index) {
        if(node->readdir != 0 && (node->flags & 0x07) == VFS_DIRECTORY) return node->readdir(node, index);
        return 0;
    }

    node_t *finddir(node_t *node, const char *name) {
        if(node->finddir != 0 && (node->flags & 0x07) == VFS_DIRECTORY) return node->finddir(node, name);
        return 0;
    }
}
