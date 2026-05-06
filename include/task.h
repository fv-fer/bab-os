#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <vmm.h>
#include <isr.h>

typedef struct task {
    int id;
    uint32_t esp;              /* Current stack pointer */
    uint32_t kstack;           /* Stack base address */
    page_directory_t *page_directory;
    struct task *next;
} task_t;

void task_init();
void task_create(void (*entry_point)());
uint32_t schedule(uint32_t current_esp);

#endif
