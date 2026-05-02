#include <kheap.h>
#include <vmm.h>
#include <pmm.h>
#include <string.h>
#include <terminal.h>

static header_t* heap_start = NULL;
static uint32_t heap_end = KHEAP_START;

void kheap_init() {
    /* Initially map 1MB for the heap */
    for (uint32_t i = 0; i < KHEAP_INITIAL_SIZE; i += PAGE_SIZE) {
        void* phys = pmm_alloc_pages(0);
        vmm_map_page(KHEAP_START + i, (uint32_t)phys, PAGE_PRESENT | PAGE_RW);
    }

    heap_start = (header_t*)KHEAP_START;
    heap_start->magic = KHEAP_MAGIC;
    heap_start->is_hole = 1;
    heap_start->size = KHEAP_INITIAL_SIZE - sizeof(header_t);
    heap_start->next = NULL;
    heap_start->prev = NULL;

    heap_end = KHEAP_START + KHEAP_INITIAL_SIZE;
}

/* Helper to split a block into two: [size] and [remainder] */
static void split_block(header_t* block, uint32_t size) {
    if (block->size <= size + sizeof(header_t)) return;

    header_t* new_hole = (header_t*)((uint32_t)block + sizeof(header_t) + size);
    new_hole->magic = KHEAP_MAGIC;
    new_hole->is_hole = 1;
    new_hole->size = block->size - size - sizeof(header_t);
    new_hole->next = block->next;
    new_hole->prev = block;

    if (block->next) block->next->prev = new_hole;
    block->next = new_hole;
    block->size = size;
}

static void expand_heap(size_t new_size) {
    if (new_size % PAGE_SIZE != 0) {
        new_size = (new_size / PAGE_SIZE + 1) * PAGE_SIZE;
    }

    uint32_t old_heap_end = heap_end;
    for (uint32_t i = 0; i < new_size; i += PAGE_SIZE) {
        void* phys = pmm_alloc_pages(0);
        vmm_map_page(heap_end + i, (uint32_t)phys, PAGE_PRESENT | PAGE_RW);
    }
    heap_end += new_size;

    header_t* last = heap_start;
    while (last->next) last = last->next;

    if (last->is_hole) {
        last->size += new_size;
    } else {
        header_t* new_hole = (header_t*)old_heap_end;
        new_hole->magic = KHEAP_MAGIC;
        new_hole->is_hole = 1;
        new_hole->size = new_size - sizeof(header_t);
        new_hole->prev = last;
        new_hole->next = NULL;
        last->next = new_hole;
    }
}

static void* kmalloc_internal(size_t size, int align) {
    header_t* curr = heap_start;

    while (curr) {
        if (!curr->is_hole) {
            curr = curr->next;
            continue;
        }

        uint32_t data_addr = (uint32_t)curr + sizeof(header_t);
        uint32_t adjusted_addr = (align && (data_addr & 0xFFF)) ? (data_addr + 0xFFF) & ~0xFFF : data_addr;
        uint32_t padding = adjusted_addr - data_addr;

        if (curr->size < padding + size) {
            curr = curr->next;
            continue;
        }

        /* If alignment requested and we can split off the padding... */
        if (padding > sizeof(header_t)) {
            split_block(curr, padding - sizeof(header_t));
            curr = curr->next; /* curr is now the aligned block */
        } else if (padding > 0) {
            /* Padding exists but too small for a header, cannot align here */
            curr = curr->next;
            continue;
        }

        /* Now curr is aligned (or no alignment needed). Split off unused space at the end. */
        split_block(curr, size);
        curr->is_hole = 0;
        return (void*)((uint32_t)curr + sizeof(header_t));
    }

    /* Expand and try again */
    expand_heap(size + (align ? PAGE_SIZE : 0) + sizeof(header_t));
    return kmalloc_internal(size, align);
}

void* kmalloc(size_t size) { return kmalloc_internal(size, 0); }
void* kmalloc_a(size_t size) { return kmalloc_internal(size, 1); }

void kfree(void* ptr) {
    if (!ptr || (uint32_t)ptr < KHEAP_START || (uint32_t)ptr >= heap_end) return;

    header_t* header = (header_t*)((uint32_t)ptr - sizeof(header_t));
    if (header->magic != KHEAP_MAGIC) {
        terminal_writestring("kfree: INVALID MAGIC!\n");
        return;
    }

    header->is_hole = 1;

    /* Merge forward */
    if (header->next && header->next->is_hole) {
        header->size += header->next->size + sizeof(header_t);
        header->next = header->next->next;
        if (header->next) header->next->prev = header;
    }

    /* Merge backward */
    if (header->prev && header->prev->is_hole) {
        header->prev->size += header->size + sizeof(header_t);
        header->prev->next = header->next;
        if (header->next) header->next->prev = header->prev;
    }
}
