#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <vbe.h>

void terminal_initialize(struct vbe_mode_info* vbe);
void terminal_set_color(uint32_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, uint32_t size);
void terminal_writestring(const char* data);
void terminal_writehex(uint32_t n);
void terminal_clear();

#endif
