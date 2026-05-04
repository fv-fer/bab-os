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

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

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
    printf("GDT, IDT, and ISRs Initialized.\n");
    printf("PMM: Buddy System 128MB at %x\n", 0x400000);
    printf("VMM: Higher-Half Kernel at %x\n", 0xC0000000);
    printf("Heap: Initialized at %x\n", KHEAP_START);
    
    // Test kmalloc and printf
    printf("Testing kmalloc...\n");
    char* test_str = (char*)kmalloc(64);
    strcpy(test_str, "Dynamic Message on Heap!");
    printf("Read from heap: \"%s\" at %x\n", test_str, test_str);
    kfree(test_str);

    void* ptr4 = kmalloc_a(128);
    printf("Aligned Allocation: %x (ends in 000)\n", ptr4);
    kfree(ptr4);

    printf("\nSystem Ready. Try typing on your keyboard!\n> ");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        sleep(5000);
        printf("\n[System Heartbeat] %d seconds have passed...\n> ", timer_get_ticks() / 100);
    }
}
