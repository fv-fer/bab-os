#include <terminal.h>
#include <font.h>

/* Using the struct definition from main.c for now */
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

static struct vbe_mode_info* terminal_vbe;
static uint32_t terminal_row;
static uint32_t terminal_column;
static uint32_t terminal_color;

static void put_pixel(int x, int y, uint32_t color) {
    uint8_t* pixel_ptr = (uint8_t*)terminal_vbe->framebuffer + (y * terminal_vbe->pitch) + (x * (terminal_vbe->bpp / 8));
    if (terminal_vbe->bpp == 32) {
        *((uint32_t*)pixel_ptr) = color;
    } else if (terminal_vbe->bpp == 24) {
        pixel_ptr[0] = (color & 0xFF);
        pixel_ptr[1] = (color >> 8) & 0xFF;
        pixel_ptr[2] = (color >> 16) & 0xFF;
    }
}

void terminal_initialize(struct vbe_mode_info* vbe) {
    terminal_vbe = vbe;
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x00FFFFFF; // White
    terminal_clear();
}

void terminal_clear() {
    for (uint32_t y = 0; y < terminal_vbe->height; y++) {
        for (uint32_t x = 0; x < terminal_vbe->width; x++) {
            put_pixel(x, y, 0x00000000); // Black
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row += 10; // 8 pixels for char + 2 for spacing
        return;
    }

    uint8_t* glyph = font8x8_basic[(int)c];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // Fix mirrored characters: bit 7 is the leftmost pixel
            if (glyph[i] & (0x80 >> j)) {
                put_pixel(terminal_column + j, terminal_row + i, terminal_color);
            }
        }
    }

    terminal_column += 8;
    if (terminal_column >= terminal_vbe->width - 8) {
        terminal_column = 0;
        terminal_row += 10;
    }
}

void terminal_writestring(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}
