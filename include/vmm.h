#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <isr.h>

#define PAGE_PRESENT  0x1
#define PAGE_RW       0x2
#define PAGE_USER     0x4

typedef uint32_t pde_t;
typedef uint32_t pte_t;

typedef struct page_directory {
    pde_t entries[1024];
} __attribute__((aligned(4096))) page_directory_t;

typedef struct page_table {
    pte_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

void vmm_init();
void vmm_map_page(uint32_t virtual, uint32_t physical, uint32_t flags);
void vmm_switch_page_directory(page_directory_t* dir);

#endif
