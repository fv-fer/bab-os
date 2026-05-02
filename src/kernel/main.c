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

    // Initialize the terminal with VBE info
    terminal_initialize(vbe);

    terminal_writestring("Bab-OS Kernel Booting...\n");
    terminal_writestring("GDT, IDT, and ISRs Initialized.\n");
    terminal_writestring("Buddy System PMM Initialized (128MB).\n");
    terminal_writestring("Higher-Half VMM Initialized.\n");
    terminal_writestring("Kernel Heap Initialized at 0xD0000000.\n");
    
    // Test kmalloc
    terminal_writestring("Testing kmalloc...\n");
    char* test_str = (char*)kmalloc(64);
    terminal_writestring("Allocated 64 bytes for string at: ");
    terminal_writehex((uint32_t)test_str);
    terminal_writestring("\n");

    // Write to heap
    const char* message = "Hello from the Kernel Heap!";
    memcpy(test_str, message, 28);

    // Read from heap
    terminal_writestring("Read back from heap: \"");
    terminal_writestring(test_str);
    terminal_writestring("\"\n");

    kfree(test_str);
    terminal_writestring("Freed string pointer.\n");

    void* ptr4 = kmalloc_a(128);
    terminal_writestring("Allocated aligned 128 bytes at: ");
    terminal_writehex((uint32_t)ptr4);
    terminal_writestring(" (should end in 000)\n");

    terminal_writestring("\nSystem Ready.\n> ");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        /* Every 5 seconds, print a heartbeat message */
        sleep(5000);
        terminal_writestring("\n[System Heartbeat] 5 seconds have passed...\n> ");
    }
}


