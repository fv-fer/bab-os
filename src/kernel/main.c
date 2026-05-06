#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <keyboard.h>
#include <timer.h>
#include <pmm.h>
#include <vmm.h>
#include <kheap.h>
#include <vbe.h>
#include <string.h>
#include <task.h>

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

void task_a() {
    while(1) {
        printf("A");
        for (volatile int i = 0; i < 1000000; i++);
    }
}

void task_b() {
    while(1) {
        printf("B");
        for (volatile int i = 0; i < 1000000; i++);
    }
}

void kmain() {
    idt_init();
    isr_install();

    /* 1. Initialize PMM Metadata */
    pmm_init(128 * 1024 * 1024, 0x400000);
    
    /* 2. Add available memory to the Buddy System */
    pmm_add_region(0x500000, (128 - 5) * 1024 * 1024);

    /* 3. Initialize Virtual Memory and Enable Paging */
    vmm_init();

    /* 4. Initialize Kernel Heap */
    kheap_init();

    timer_init(100); // 100 Hz
    keyboard_init();

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    terminal_initialize(vbe);

    printf("Bab-OS Kernel Booting...\n");
    
    /* 5. Initialize Multitasking */
    task_init();
    task_create(task_a);
    task_create(task_b);

    printf("System Ready. Multitasking active!\n> ");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        __asm__ volatile("hlt");
    }
}
