#include <vmm.h>
#include <pmm.h>
#include <terminal.h>
#include <string.h>
#include <vbe.h>

static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

extern void isr_handler(registers_t *r);

void page_fault_handler(registers_t *r) {
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(r->err_code & 0x1);
    int rw = r->err_code & 0x2;
    int us = r->err_code & 0x4;
    int reserved = r->err_code & 0x8;
    int id = r->err_code & 0x10;

    terminal_writestring("\nPAGE FAULT! (");
    if (present) terminal_writestring("present ");
    if (rw) terminal_writestring("read-only ");
    if (us) terminal_writestring("user-mode ");
    if (reserved) terminal_writestring("reserved ");
    terminal_writestring(") at ");
    terminal_writehex(faulting_address);
    
    terminal_writestring("\nKernel Halted!\n");
    for (;;);
}

void vmm_init() {
    /* 1. Allocate a page for the kernel directory */
    kernel_directory = (page_directory_t*)pmm_alloc_pages(0);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;

    /* 2. Identity map the first 8MB to cover Kernel, PMM metadata, etc. */
    /* 8MB = 2048 pages */
    for (uint32_t i = 0; i < 0x800000; i += PAGE_SIZE) {
        vmm_map_page(i, i, PAGE_PRESENT | PAGE_RW);
    }

    /* 3. Identity map the VBE Framebuffer */
    /* We need to get the framebuffer address from the VBE info at 0x8000 */
    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    uint32_t fb_start = vbe->framebuffer;
    uint32_t fb_size = vbe->height * vbe->pitch;

    for (uint32_t i = 0; i < fb_size; i += PAGE_SIZE) {
        vmm_map_page(fb_start + i, fb_start + i, PAGE_PRESENT | PAGE_RW);
    }

    /* 4. Register Page Fault Handler */
    register_interrupt_handler(14, page_fault_handler);

    /* 5. Switch to the new directory and Enable Paging */
    vmm_switch_page_directory(kernel_directory);

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Enable PG bit
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

void vmm_switch_page_directory(page_directory_t* dir) {
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" :: "r"(dir));
}

void vmm_map_page(uint32_t virtual, uint32_t physical, uint32_t flags) {
    uint32_t pd_index = virtual >> 22;
    uint32_t pt_index = (virtual >> 12) & 0x3FF;

    pde_t* pde = &current_directory->entries[pd_index];
    page_table_t* pt;

    if (!(*pde & PAGE_PRESENT)) {
        /* Page table not present, allocate one */
        pt = (page_table_t*)pmm_alloc_pages(0);
        memset(pt, 0, sizeof(page_table_t));
        *pde = (uint32_t)pt | flags | PAGE_PRESENT;
    } else {
        /* Page table is present, but it's just a physical address in the PDE */
        /* In this basic kernel, physical == virtual for the page tables themselves */
        /* because we identity mapped everything where we allocate them. */
        pt = (page_table_t*)(*pde & 0xFFFFF000);
    }

    pt->entries[pt_index] = (physical & 0xFFFFF000) | flags;
    
    /* Invalidate TLB */
    __asm__ volatile("invlpg (%0)" :: "r"(virtual) : "memory");
}
