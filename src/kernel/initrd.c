#include <initrd.h>
#include <kheap.h>
#include <string.h>

initrd_header_t *initrd_header;     /* The main header */
initrd_file_header_t *file_headers; /* Array of file headers */
vfs_node_t *initrd_root;            /* Root directory node */
vfs_node_t *initrd_dev;             /* We also add a dev directory */
vfs_node_t *root_nodes;             /* List of file nodes */
int nroot_nodes;                    /* Number of file nodes */

struct vfs_dirent dirent;

static uint32_t initrd_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length)
        return 0;
    if (offset+size > header.length)
        size = header.length - offset;
    memcpy(buffer, (uint8_t*) (header.offset + offset), size);
    return size;
}

static struct vfs_dirent *initrd_readdir(vfs_node_t *node, uint32_t index) {
    if (node == initrd_root && index == 0) {
      strcpy(dirent.name, "dev");
      dirent.inode = 0;
      return &dirent;
    }

    if (node == initrd_root && index-1 < nroot_nodes) {
      strcpy(dirent.name, root_nodes[index-1].name);
      dirent.inode = root_nodes[index-1].inode;
      return &dirent;
    }

    return NULL;
}

static vfs_node_t *initrd_finddir(vfs_node_t *node, const char *name) {
    if (node == initrd_root && !strcmp(name, "dev"))
        return initrd_dev;

    int i;
    for (i = 0; i < nroot_nodes; i++)
        if (!strcmp(name, root_nodes[i].name))
            return &root_nodes[i];
    return NULL;
}

vfs_node_t *initialise_initrd(uint32_t location) {
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *)(location + sizeof(initrd_header_t));

    initrd_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    strcpy(initrd_root->name, "initrd");
    initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
    initrd_root->flags = VFS_DIRECTORY;
    initrd_root->read = 0;
    initrd_root->write = 0;
    initrd_root->open = 0;
    initrd_root->close = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr = 0;
    initrd_root->impl = 0;

    initrd_dev = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    strcpy(initrd_dev->name, "dev");
    initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
    initrd_dev->flags = VFS_DIRECTORY;
    initrd_dev->read = 0;
    initrd_dev->write = 0;
    initrd_dev->open = 0;
    initrd_dev->close = 0;
    initrd_dev->readdir = &initrd_readdir;
    initrd_dev->finddir = &initrd_finddir;
    initrd_dev->ptr = 0;
    initrd_dev->impl = 0;

    root_nodes = (vfs_node_t*)kmalloc(sizeof(vfs_node_t) * initrd_header->nfiles);
    nroot_nodes = initrd_header->nfiles;

    int i;
    for (i = 0; i < initrd_header->nfiles; i++) {
        file_headers[i].offset += location;
        strcpy(root_nodes[i].name, file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = VFS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].impl = 0;
    }
    
    return initrd_root;
}
