#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>

/* Start the heap at 3.25 GB (just above the higher-half kernel) */
#define KHEAP_START         0xD0000000
#define KHEAP_INITIAL_SIZE  0x100000    /* 1 MB */
#define KHEAP_MAX_ADDRESS   0xEFFFF000  /* Up to 3.75 GB */

typedef struct header {
    uint32_t magic;
    uint8_t  is_hole;
    uint32_t size;
    struct header* next;
    struct header* prev;
} header_t;

#define KHEAP_MAGIC 0x12345678

void  kheap_init();
void* kmalloc(size_t size);
void* kmalloc_a(size_t size); /* Aligned */
void  kfree(void* ptr);

#endif
