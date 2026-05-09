#ifndef MONITOR_H
#define MONITOR_H

#include <mutex.h>
#include <cond.h>

#define MAX_MONITOR_CONDS 8

typedef struct monitor {
    mutex_t lock;
    cond_t conds[MAX_MONITOR_CONDS];
} monitor_t;

void monitor_init(monitor_t *m);
void monitor_enter(monitor_t *m);
void monitor_exit(monitor_t *m);
void monitor_wait(monitor_t *m, int cond_idx);
void monitor_notify(monitor_t *m, int cond_idx);
void monitor_notify_all(monitor_t *m, int cond_idx);

#endif
