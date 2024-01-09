#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ECS_COMPONENT_DECLARE(Logger);

Logger *init_logger(Logger* l) {
    l->head = 0;
    return l;
}

// TODO: This function truncates long messages...
void log_msg(Logger *l, char *fmt, ...) {
    static char msgbuf[MAX_LOG_MSG_LEN];

    va_list va;
    va_start(va, fmt);
    vsnprintf(msgbuf, MAX_LOG_MSG_LEN, fmt, va);
    msgbuf[MAX_LOG_MSG_LEN - 1] = '\0';
    va_end(va);

    strncpy(l->msgs[l->head], msgbuf, MAX_LOG_MSG_LEN);
    l->head = (l->head + 1) & (MAX_LOG_MSGS - 1);
}

const char *get_last_log_msg(const Logger *l) {
    return l->msgs[(l->head - 1) & (MAX_LOG_MSGS - 1)];
}
