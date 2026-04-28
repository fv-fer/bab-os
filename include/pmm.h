#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define MAX_ORDER 12  /* Up to 2^11 * 4096 = 8MB blocks */

typedef struct page {
    uint8_t order;
    uint8_t flags; /* 0: free, 1: used */
    struct page *next;
    struct page *prev;
} page_t;

void pmm_init(size_t mem_size, uint32_t metadata_start);
void* pmm_alloc_pages(uint32_t order);
void pmm_free_pages(void* ptr, uint32_t order);

/* Legacy/Helper for single blocks */
void* pmm_alloc_block();
void pmm_free_block(void* ptr);

#endif
