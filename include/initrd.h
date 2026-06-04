#ifndef INITRD_H
#define INITRD_H

#include <stdint.h>
#include <vfs.h>

typedef struct {
    uint32_t nfiles; /* Number of files in the ramdisk */
} initrd_header_t;

typedef struct {
    uint32_t magic;  /* Magic number for identification */
    char name[64];   /* Filename */
    uint32_t offset; /* Offset in the initrd image */
    uint32_t length; /* Length of file */
} initrd_file_header_t;

/* Initialises the initial ramdisk. It gets passed the address of the multiboot module,
 * and returns a completed vfs_node_t for the / volume. */
vfs_node_t* initialise_initrd(uint32_t location);

#endif
