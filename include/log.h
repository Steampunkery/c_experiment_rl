#pragma once
#include "rogue.h"
#include <stddef.h>

#define MAX_LOG_MSGS 128
#define MAX_LOG_MSG_LEN MIN_TERM_COLS

typedef struct Logger {
    size_t head;
    wchar_t msgs[MAX_LOG_MSGS][MAX_LOG_MSG_LEN];
} Logger;

Logger *init_logger(Logger *l);
void log_msg(Logger *l, wchar_t *fmt, ...);
const wchar_t *get_last_log_msg(const Logger *l);
