#include <terminal.h>
#include <font.h>
#include <vbe.h>

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

void terminal_set_color(uint32_t color) {
    terminal_color = color;
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

void terminal_write(const char* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}

void terminal_writehex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'x';
    for (int i = 0; i < 8; i++) {
        buffer[9 - i] = hex_chars[(n >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    terminal_writestring(buffer);
}
