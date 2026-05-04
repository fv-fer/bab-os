#include <terminal.h>
#include <font.h>
#include <vbe.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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
        memset((void*)((uint32_t)terminal_vbe->framebuffer + y * terminal_vbe->pitch), 0, terminal_vbe->width * (terminal_vbe->bpp / 8));
    }
}

void terminal_scroll() {
    uint32_t row_height = 10;
    void* fb = (void*)terminal_vbe->framebuffer;
    uint32_t pitch = terminal_vbe->pitch;
    uint32_t screen_height = terminal_vbe->height;

    /* Move everything up by row_height */
    memcpy(fb, (void*)((uint32_t)fb + row_height * pitch), (screen_height - row_height) * pitch);

    /* Clear the last row */
    for (uint32_t y = screen_height - row_height; y < screen_height; y++) {
        memset((void*)((uint32_t)fb + y * pitch), 0, terminal_vbe->width * (terminal_vbe->bpp / 8));
    }

    terminal_row -= row_height;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row += 10;
        if (terminal_row >= terminal_vbe->height - 10) {
            terminal_scroll();
        }
        return;
    }

    uint8_t* glyph = font8x8_basic[(int)c];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                put_pixel(terminal_column + j, terminal_row + i, terminal_color);
            }
        }
    }

    terminal_column += 8;
    if (terminal_column >= terminal_vbe->width - 8) {
        terminal_column = 0;
        terminal_row += 10;
        if (terminal_row >= terminal_vbe->height - 10) {
            terminal_scroll();
        }
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

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 's') {
                char* s = va_arg(args, char*);
                terminal_writestring(s);
            } else if (*format == 'd') {
                int d = va_arg(args, int);
                char buf[32];
                itoa(d, buf, 10);
                terminal_writestring(buf);
            } else if (*format == 'x') {
                uint32_t x = va_arg(args, uint32_t);
                char buf[32];
                itoa(x, buf, 16);
                terminal_writestring("0x");
                terminal_writestring(buf);
            } else if (*format == 'c') {
                char c = (char)va_arg(args, int);
                terminal_putchar(c);
            } else if (*format == '%') {
                terminal_putchar('%');
            }
        } else {
            terminal_putchar(*format);
        }
        format++;
    }

    va_end(args);
}
