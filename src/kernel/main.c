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
#include <monitor.h>

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

#define BUFFER_SIZE 5
#define COND_NOT_EMPTY 0
#define COND_NOT_FULL  1

monitor_t buffer_monitor;
int buffer[BUFFER_SIZE];
int count = 0;
int head = 0;
int tail = 0;

void producer() {
    int item = 100;
    while(1) {
        sleep(300); // Produce quickly

        monitor_enter(&buffer_monitor);
        
        while (count == BUFFER_SIZE) {
            printf("[P: buffer full, waiting...]");
            monitor_wait(&buffer_monitor, COND_NOT_FULL);
        }

        buffer[tail] = item++;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;
        
        printf("[P: produced, count %d]", count);
        
        monitor_notify(&buffer_monitor, COND_NOT_EMPTY);
        monitor_exit(&buffer_monitor);
    }
}

void consumer() {
    while(1) {
        sleep(800); // Consume slowly to force buffer to fill up

        monitor_enter(&buffer_monitor);

        while (count == 0) {
            printf("[C: buffer empty, waiting...]");
            monitor_wait(&buffer_monitor, COND_NOT_EMPTY);
        }

        int item = buffer[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;

        printf("[C: consumed %d, count %d]", item, count);

        monitor_notify(&buffer_monitor, COND_NOT_FULL);
        monitor_exit(&buffer_monitor);
    }
}

void shell_task() {
    printf("Shell started. Type something!\n> ");
    while(1) {
        char c = getchar();
        /* The keyboard driver already echoes characters to the terminal.
           We just handle the new line prompt here. */
        if (c == '\n') {
            printf("> ");
        }
    }
}

void kmain() {
    gdt_init();
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

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0xC0000500;
    terminal_initialize(vbe);

    printf("Bab-OS Kernel Booting...\n");

    keyboard_init();

    monitor_init(&buffer_monitor);

    /* 5. Initialize Multitasking */
    task_init();
    task_create(producer);
    task_create(consumer);
    task_create(shell_task);

    printf("System Ready. Multitasking active!\n");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        __asm__ volatile("hlt");
    }
}
