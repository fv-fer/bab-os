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

void kmain() {
    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    uint32_t* framebuffer = (uint32_t*) vbe->framebuffer;
    uint16_t width = vbe->width;
    uint16_t height = vbe->height;

    // Fill the screen with Blue (ARGB: 0xFF0000FF)
    for (uint32_t i = 0; i < width * height; i++) {
        framebuffer[i] = 0x000000FF;
    }

    // Draw a small Red square in the middle
    int centerX = width / 2;
    int centerY = height / 2;
    int size = 50;
    for (int y = centerY - size; y < centerY + size; y++) {
        for (int x = centerX - size; x < centerX + size; x++) {
            framebuffer[y * width + x] = 0x00FF0000;
        }
    }

    while(1);
}

