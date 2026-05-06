#include <task.h>
#include <kheap.h>
#include <pmm.h>
#include <string.h>
#include <terminal.h>

static task_t *running_task = NULL;
static task_t *ready_queue = NULL;
static int next_task_id = 1;

extern page_directory_t* current_directory;

void task_init() {
    /* Initialize the first task (the current kernel execution) */
    running_task = (task_t*)kmalloc(sizeof(task_t));
    running_task->id = next_task_id++;
    running_task->esp = 0;
    running_task->kstack = 0;
    running_task->page_directory = current_directory;
    running_task->next = NULL;
    
    ready_queue = running_task;
    
    printf("Multitasking initialized. Initial task ID: %d\n", running_task->id);
}

void task_create(void (*entry_point)()) {
    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    new_task->id = next_task_id++;
    
    /* Allocate 4KB stack */
    uint32_t stack_base = (uint32_t)kmalloc(4096);
    uint32_t stack_top = stack_base + 4096;
    new_task->kstack = stack_base;
    
    uint32_t *ptr = (uint32_t*)stack_top;
    
    /* 
     * Initial stack for a kernel task (Ring 0 -> Ring 0):
     * [esp + 12] EFLAGS
     * [esp + 8]  CS
     * [esp + 4]  EIP
     * [esp + 0]  error code
     * [esp - 4]  int_no
     * ... pusha ...
     * [esp - 36] ds
     */

    *(--ptr) = 0x0202;        /* eflags (interrupts enabled) */
    *(--ptr) = 0x08;          /* cs */
    *(--ptr) = (uint32_t)entry_point; /* eip */
    
    *(--ptr) = 0;             /* error code */
    *(--ptr) = 0;             /* int_no */
    
    /* pusha (eax, ecx, edx, ebx, esp, ebp, esi, edi) */
    *(--ptr) = 0; /* eax */
    *(--ptr) = 0; /* ecx */
    *(--ptr) = 0; /* edx */
    *(--ptr) = 0; /* ebx */
    *(--ptr) = 0; /* esp */
    *(--ptr) = 0; /* ebp */
    *(--ptr) = 0; /* esi */
    *(--ptr) = 0; /* edi */
    
    *(--ptr) = 0x10;          /* ds */
    
    new_task->esp = (uint32_t)ptr;
    new_task->page_directory = current_directory;
    new_task->next = NULL;
    
    /* Add to ready queue */
    task_t *tmp = ready_queue;
    while(tmp->next) tmp = tmp->next;
    tmp->next = new_task;
    
    printf("Created task %d at %x, esp %x\n", new_task->id, entry_point, new_task->esp);
}

uint32_t schedule(uint32_t current_esp) {
    if (!running_task) return current_esp;
    
    /* Save current ESP */
    running_task->esp = current_esp;
    
    /* Pick next task (Round Robin) */
    running_task = running_task->next;
    if (!running_task) running_task = ready_queue;
    
    return running_task->esp;
}
