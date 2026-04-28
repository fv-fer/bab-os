#include <isr.h>
#include <io.h>
#include <terminal.h>
#include <keyboard.h>
#include <stdint.h>
#include <stdbool.h>

#define KBD_DATA_PORT 0x60

/* State variables */
static bool left_shift_pressed = false;
static bool right_shift_pressed = false;
static bool caps_lock = false;

/* US Keyboard Layout - Normal */
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift (0x2A) */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift (0x36) */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock (0x3A) */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

/* US Keyboard Layout - Shifted */
unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = inb(KBD_DATA_PORT);

    /* Handle Key Releases */
    if (scancode & 0x80) {
        uint8_t released_scancode = scancode & 0x7F;
        if (released_scancode == 0x2A) left_shift_pressed = false;
        if (released_scancode == 0x36) right_shift_pressed = false;
        return;
    }

    /* Handle Key Presses */
    switch (scancode) {
        case 0x2A: /* Left Shift */
            left_shift_pressed = true;
            break;
        case 0x36: /* Right Shift */
            right_shift_pressed = true;
            break;
        case 0x3A: /* Caps Lock */
            caps_lock = !caps_lock;
            break;
        default: {
            bool shift = left_shift_pressed || right_shift_pressed;
            char c = 0;

            /* Logic for combining Shift and Caps Lock */
            if (shift) {
                c = kbd_us_shift[scancode];
            } else {
                c = kbd_us[scancode];
            }

            /* Caps Lock only affects alphabetical characters */
            if (caps_lock && c >= 'a' && c <= 'z') {
                if (!shift) c = kbd_us_shift[scancode]; // Uppercase if CapsLock is on but Shift is NOT
            } else if (caps_lock && c >= 'A' && c <= 'Z') {
                if (shift) c = kbd_us[scancode]; // Lowercase if BOTH are on (toggles back)
            }

            if (c > 0) {
                terminal_putchar(c);
            }
            break;
        }
    }
}

void keyboard_init() {
    register_interrupt_handler(33, keyboard_callback);
}
