#include <stdlib.h>
#include <stdint.h>

int abs(int value) {
    return value < 0 ? -value : value;
}

char* itoa(int value, char* str, int base) {
    char *rc;
    char *ptr;
    char *low;
    // Check for supported base
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative numbers and work with absolute value
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }
    low = ptr;
    uint32_t v = (value < 0 && base == 10) ? (uint32_t)-value : (uint32_t)value;
    // Special case for 0
    if (v == 0) {
        *ptr++ = '0';
    } else {
        // Extract digits in reverse order
        while (v > 0) {
            uint32_t digit = v % base;
            *ptr++ = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
            v /= base;
        }
    }
    // Terminate string
    *ptr-- = '\0';
    // Reverse digits
    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}
