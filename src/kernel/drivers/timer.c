#include <timer.h>
#include <isr.h>
#include <io.h>
#include <terminal.h>

uint32_t tick = 0;

static void timer_callback(registers_t *regs) {
    tick++;
}

void timer_init(uint32_t frequency) {
    /* Register our timer handler */
    register_interrupt_handler(32, timer_callback);

    /* The value we send to the PIT is the value to divide it's input clock
    * (1193180 Hz) by, to get our required frequency */
    uint32_t divisor = 1193180 / frequency;

    /* Send the command byte */
    outb(0x43, 0x36);

    /* Divisor has to be sent byte-wise, so split here into upper/lower bytes */
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor >> 8) & 0xFF );

    /* Send the frequency divisor */
    outb(0x40, l);
    outb(0x40, h);
}

uint32_t timer_get_ticks() {
    return tick;
}

void sleep(uint32_t ms) {
    uint32_t start_ticks = tick;
    /* Since timer is 100Hz, 1 tick = 10ms */
    uint32_t ticks_to_wait = ms / 10;
    
    while(tick < (start_ticks + ticks_to_wait)) {
        /* Busy wait until the required number of ticks has passed */
        /* We use 'hlt' to save power/CPU cycles while waiting */
        __asm__ volatile("hlt");
    }
}
