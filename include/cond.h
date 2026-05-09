#ifndef COND_H
#define COND_H

#include <task.h>
#include <mutex.h>

typedef struct cond {
    task_t *waiters;
} cond_t;

void cond_init(cond_t *c);
void cond_wait(cond_t *c, mutex_t *m);
void cond_signal(cond_t *c);
void cond_broadcast(cond_t *c);

#endif
