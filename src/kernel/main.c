#include <stdint.h>

// VBE Mode Info Structure (partial)
struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch; // bytes per scanline
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

    uint32_t framebuffer; // Physical address of the linear frame buffer
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed));

void put_pixel(struct vbe_mode_info* vbe, int x, int y, uint32_t color) {
    // 1. Calculate the starting byte of the specific pixel.
    // We use 'vbe->pitch' for the Y math and 'bpp/8' for the X math.
    uint8_t* pixel_ptr = (uint8_t*)vbe->framebuffer + (y * vbe->pitch) + (x * (vbe->bpp / 8));

    if (vbe->bpp == 32) {
        // 32-bit mode is easy: just cast to a 32-bit pointer and write
        *((uint32_t*)pixel_ptr) = color;
    } 
    else if (vbe->bpp == 24) {
        // 24-bit mode: We must write 3 individual bytes.
        // Standard VESA order is Blue, Green, Red (BGR)
        pixel_ptr[0] = (color & 0xFF);         // Blue component
        pixel_ptr[1] = (color >> 8) & 0xFF;    // Green component
        pixel_ptr[2] = (color >> 16) & 0xFF;   // Red component
    }
}

void kmain() {
    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    
    // Safety check: Is the framebuffer address even valid?
    if (vbe->framebuffer == 0) return;

    // Clear screen using the dynamic function
    for (int y = 0; y < vbe->height; y++) {
        for (int x = 0; x < vbe->width; x++) {
            put_pixel(vbe, x, y, 0x000000FF); // Fill Blue
        }
    }
    
    // Draw the Red square
    for (int y = 300; y < 400; y++) {
        for (int x = 300; x < 400; x++) {
            put_pixel(vbe, x, y, 0x00FF0000); // Draw Red
        }
    }

    while(1);
}
