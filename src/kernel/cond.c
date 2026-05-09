#include <cond.h>
#include <io.h>
#include <stddef.h>

void cond_init(cond_t *c) {
    c->waiters = NULL;
}

void cond_wait(cond_t *c, mutex_t *m) {
    cli();
    
    task_t *current = task_get_current();
    
    /* Add current task to the condition variable's wait queue */
    current->state = TASK_BLOCKED;
    current->wait_next = NULL;
    
    if (!c->waiters) {
        c->waiters = current;
    } else {
        task_t *tmp = c->waiters;
        while (tmp->wait_next) tmp = tmp->wait_next;
        tmp->wait_next = current;
    }
    
    /* Release the mutex. This must be done while interrupts are disabled
     * to prevent a race condition where we are unblocked before we yield. */
    mutex_unlock(m);
    
    /* Yield control. Note: mutex_unlock re-enabled interrupts, but that's okay
     * because we already marked ourselves as BLOCKED. */
    task_yield();
    
    /* Once woken up, we must re-acquire the mutex before returning */
    mutex_lock(m);
}

void cond_signal(cond_t *c) {
    cli();
    
    if (c->waiters) {
        task_t *waiter = c->waiters;
        c->waiters = waiter->wait_next;
        
        /* Mark the task as ready so the scheduler can pick it */
        waiter->state = TASK_READY;
    }
    
    sti();
}

void cond_broadcast(cond_t *c) {
    cli();
    
    while (c->waiters) {
        task_t *waiter = c->waiters;
        c->waiters = waiter->wait_next;
        waiter->state = TASK_READY;
    }
    
    sti();
}
