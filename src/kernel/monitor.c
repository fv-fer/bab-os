#include <monitor.h>

void monitor_init(monitor_t *m) {
    mutex_init(&m->lock);
    for (int i = 0; i < MAX_MONITOR_CONDS; i++) {
        cond_init(&m->conds[i]);
    }
}

void monitor_enter(monitor_t *m) {
    mutex_lock(&m->lock);
}

void monitor_exit(monitor_t *m) {
    mutex_unlock(&m->lock);
}

void monitor_wait(monitor_t *m, int cond_idx) {
    if (cond_idx >= 0 && cond_idx < MAX_MONITOR_CONDS) {
        cond_wait(&m->conds[cond_idx], &m->lock);
    }
}

void monitor_notify(monitor_t *m, int cond_idx) {
    if (cond_idx >= 0 && cond_idx < MAX_MONITOR_CONDS) {
        cond_signal(&m->conds[cond_idx]);
    }
}

void monitor_notify_all(monitor_t *m, int cond_idx) {
    if (cond_idx >= 0 && cond_idx < MAX_MONITOR_CONDS) {
        cond_broadcast(&m->conds[cond_idx]);
    }
}
