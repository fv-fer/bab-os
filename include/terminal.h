#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

// We'll reuse the vbe_mode_info struct definition
struct vbe_mode_info;

void terminal_initialize(struct vbe_mode_info* vbe);
void terminal_set_color(uint32_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, uint32_t size);
void terminal_writestring(const char* data);
void terminal_clear();

#endif
