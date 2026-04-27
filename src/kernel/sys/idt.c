#include <idt.h>
#include <string.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

/* Extern function defined in idt_load.asm */
extern void idt_load(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;

    /* Initialize all gates to zero/empty */
    // Note: We'll need memset here, which we haven't implemented yet.
    // For now, I'll use a simple loop or implement memset in common.
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Load the IDT */
    idt_load((uint32_t)&idtp);
}
