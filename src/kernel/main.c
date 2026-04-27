#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>

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

void kmain() {
    gdt_init();
    idt_init();
    isr_install();

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    
    // Initialize the terminal with VBE info
    terminal_initialize(vbe);

    // Now we can print easily!
    terminal_writestring("Bab-OS Kernel Booting...\n");
    terminal_writestring("GDT, IDT, and ISRs Initialized.\n");
    terminal_writestring("VBE Graphics Initialized: 1024x768x32\n");
    terminal_writestring("Terminal Driver Loaded Successfully.\n\n");
    
    terminal_writestring("Welcome to your new OS!");

    // Test exception (division by zero)
    //     int i = 1 / 0;

    while(1);
}
