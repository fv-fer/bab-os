#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <keyboard.h>
#include <timer.h>
#include <pmm.h>
#include <vmm.h>
#include <kheap.h>
#include <vbe.h>
#include <string.h>
#include <task.h>
#include <mutex.h>
#include <cond.h>

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

mutex_t buffer_mutex;
cond_t buffer_cond;
int buffer_count = 0;

void producer() {
    int item = 0;
    while(1) {
        sleep(500); // Produce every 500ms

        mutex_lock(&buffer_mutex);
        item = item + 10;
        buffer_count = buffer_count + 10;
        printf("[P: produced %d, count %d]", item, buffer_count);
        cond_signal(&buffer_cond);
        mutex_unlock(&buffer_mutex);
    }
}

void consumer() {
    while(1) {
        mutex_lock(&buffer_mutex);

        while (buffer_count == 0) {
            printf("[C: empty, waiting...]");
            cond_wait(&buffer_cond, &buffer_mutex);
        }

        buffer_count--;
        printf("[C: consumed, count %d]", buffer_count);

        mutex_unlock(&buffer_mutex);

        // Simulate processing time
        sleep(200);
    }
}

void kmain() {
    idt_init();
    isr_install();

    /* 1. Initialize PMM Metadata */
    pmm_init(128 * 1024 * 1024, 0x400000);

    /* 2. Add available memory to the Buddy System */
    pmm_add_region(0x500000, (128 - 5) * 1024 * 1024);

    /* 3. Initialize Virtual Memory and Enable Paging */
    vmm_init();

    /* 4. Initialize Kernel Heap */
    kheap_init();

    timer_init(100); // 100 Hz
    keyboard_init();

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0x8000;
    terminal_initialize(vbe);

    printf("Bab-OS Kernel Booting...\n");

    mutex_init(&buffer_mutex);
    cond_init(&buffer_cond);

    /* 5. Initialize Multitasking */
    task_init();
    task_create(producer);
    task_create(consumer);

    printf("System Ready. Multitasking active!\n> ");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        __asm__ volatile("hlt");
    }
}
