#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void isr_install();
uint32_t isr_handler(registers_t *r);
uint32_t irq_handler(registers_t *r);

typedef uint32_t (*isr_t)(registers_t *);
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif
