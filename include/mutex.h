#ifndef MUTEX_H
#define MUTEX_H

#include <task.h>

typedef struct mutex {
    int locked;
    task_t *owner;
    task_t *waiters;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

#endif
