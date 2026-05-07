#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <vmm.h>
#include <isr.h>

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED
} task_state_t;

typedef struct task {
    int id;
    uint32_t esp;              /* Current stack pointer */
    uint32_t kstack;           /* Stack base address */
    page_directory_t *page_directory;
    task_state_t state;
    struct task *next;
    struct task *wait_next;    /* Next task in a wait queue */
} task_t;

void task_init();
void task_create(void (*entry_point)());
uint32_t schedule(uint32_t current_esp);
void task_yield();
task_t* task_get_current();

#endif
