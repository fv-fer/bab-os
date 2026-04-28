#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <keyboard.h>
#include <timer.h>
#include <pmm.h>

// VBE Mode Info Structure
struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed));

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

void kmain() {
    gdt_init();
    idt_init();
    isr_install();

    /* 1. Initialize PMM Metadata */
    /* Assume 128MB RAM. Metadata needs (128MB / 4KB) * sizeof(page_t) */
    /* 32768 pages * 16 bytes = 512KB. Let's put metadata at 4MB */
    pmm_init(128 * 1024 * 1024, 0x400000);
    
    /* 2. Add available memory to the Buddy System */
    /* We free everything from 5MB to 128MB */
    /* This automatically excludes the Kernel (0x1000) and PMM Metadata (0x400000) */
    pmm_add_region(0x500000, (128 - 5) * 1024 * 1024);

    timer_init(100); // 100 Hz
    keyboard_init();

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;

    // Initialize the terminal with VBE info
    terminal_initialize(vbe);

    // Now we can print easily!
    terminal_writestring("Bab-OS Kernel Booting...\n");
    terminal_writestring("GDT, IDT, and ISRs Initialized.\n");
    terminal_writestring("Buddy System PMM Initialized (128MB).\n");
    terminal_writestring("System Timer Initialized (100Hz).\n");
    terminal_writestring("Keyboard Driver Loaded.\n");
    terminal_writestring("VBE Graphics Initialized: 1024x768x32\n");
    terminal_writestring("Terminal Driver Loaded Successfully.\n\n");

    terminal_writestring("Welcome to your new OS!\n");
    terminal_writestring("You can now type anything:\n> ");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        /* Every 5 seconds, print a heartbeat message */
        sleep(5000);
        terminal_writestring("\n[System Heartbeat] 5 seconds have passed...\n> ");
    }
}


