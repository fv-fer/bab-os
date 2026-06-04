#include <vfs.h>
#include <string.h>

vfs_node_t* fs_root = NULL;

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

void vfs_open(vfs_node_t* node, uint8_t read, uint8_t write) {
    if (node->open) {
        node->open(node);
    }
}

void vfs_close(vfs_node_t* node) {
    if (node->close) {
        node->close(node);
    }
}

struct vfs_dirent* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if ((node->flags & 0x07) == VFS_DIRECTORY && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if ((node->flags & 0x07) == VFS_DIRECTORY && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}
