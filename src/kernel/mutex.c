#include <mutex.h>
#include <io.h>
#include <task.h>
#include <stddef.h>

void mutex_init(mutex_t *m) {
    m->locked = 0;
    m->owner = NULL;
    m->waiters = NULL;
}

void mutex_lock(mutex_t *m) {
    cli();
    
    task_t *current = task_get_current();
    
    while (m->locked) {
        /* Add current task to waiters */
        current->state = TASK_BLOCKED;
        current->wait_next = NULL;
        
        if (!m->waiters) {
            m->waiters = current;
        } else {
            task_t *tmp = m->waiters;
            while (tmp->wait_next) tmp = tmp->wait_next;
            tmp->wait_next = current;
        }
        
        task_yield();
        /* When we return here, we must re-disable interrupts because yield enabled them */
        cli();
    }

    m->locked = 1;
    m->owner = current;
    sti();
}

void mutex_unlock(mutex_t *m) {
    cli();
    
    m->locked = 0;
    m->owner = NULL;

    if (m->waiters) {
        /* Unblock the first waiter */
        task_t *waiter = m->waiters;
        m->waiters = waiter->wait_next;
        waiter->state = TASK_READY;
    }
    
    sti();
}
