#include <pmm.h>
#include <string.h>

static page_t* pages;
static uint32_t total_pages;
static page_t* free_lists[MAX_ORDER];

/* Internal Linked List Helpers */
static void list_add(page_t** head, page_t* node) {
    node->next = *head;
    node->prev = NULL;
    if (*head) (*head)->prev = node;
    *head = node;
}

static void list_remove(page_t** head, page_t* node) {
    if (node->prev) node->prev->next = node->next;
    else *head = node->next;
    if (node->next) node->next->prev = node->prev;
}

void pmm_init(size_t mem_size, uint32_t metadata_start) {
    total_pages = mem_size / PAGE_SIZE;
    pages = (page_t*)metadata_start;

    /* Initialize all page structures */
    for (uint32_t i = 0; i < total_pages; i++) {
        pages[i].order = 0;
        pages[i].flags = 1; /* Mark all as used initially */
        pages[i].next = pages[i].prev = NULL;
    }

    /* Initialize free lists */
    for (int i = 0; i < MAX_ORDER; i++) {
        free_lists[i] = NULL;
    }
}

/* Add a memory region to the buddy system */
void pmm_add_region(uint32_t start, uint32_t length) {
    uint32_t start_page = start / PAGE_SIZE;
    uint32_t end_page = (start + length) / PAGE_SIZE;

    for (uint32_t i = start_page; i < end_page; ) {
        /* Find the largest power-of-two block that fits this range and alignment */
        uint32_t order = 0;
        while (order < MAX_ORDER - 1) {
            uint32_t block_size = 1 << (order + 1);
            if (i + block_size > end_page || (i % block_size) != 0) break;
            order++;
        }

        pages[i].order = order;
        pages[i].flags = 0;
        list_add(&free_lists[order], &pages[i]);
        i += (1 << order);
    }
}

void* pmm_alloc_pages(uint32_t order) {
    for (uint32_t i = order; i < MAX_ORDER; i++) {
        if (free_lists[i]) {
            page_t* page = free_lists[i];
            list_remove(&free_lists[i], page);

            /* Split blocks if we found a larger one than needed */
            while (i > order) {
                i--;
                uint32_t buddy_idx = (page - pages) + (1 << i);
                page_t* buddy = &pages[buddy_idx];
                buddy->order = i;
                buddy->flags = 0;
                list_add(&free_lists[i], buddy);
            }

            page->flags = 1;
            page->order = order;
            return (void*)((page - pages) * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_pages(void* ptr, uint32_t order) {
    uint32_t page_idx = (uint32_t)ptr / PAGE_SIZE;
    page_t* page = &pages[page_idx];

    while (order < MAX_ORDER - 1) {
        uint32_t buddy_idx = page_idx ^ (1 << order);
        page_t* buddy = &pages[buddy_idx];

        /* Can only merge if buddy is free, same order, and within bounds */
        if (buddy_idx >= total_pages || buddy->flags == 1 || buddy->order != order) {
            break;
        }

        /* Merge with buddy */
        list_remove(&free_lists[order], buddy);
        if (buddy_idx < page_idx) {
            page_idx = buddy_idx;
            page = buddy;
        }
        order++;
    }

    page->order = order;
    page->flags = 0;
    list_add(&free_lists[order], page);
}

/* Legacy Helpers */
void* pmm_alloc_block() { return pmm_alloc_pages(0); }
void pmm_free_block(void* ptr) { pmm_free_pages(ptr, 0); }
