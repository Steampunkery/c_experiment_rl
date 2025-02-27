#pragma once
#include "rogue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_LOG_MSGS 128
#define MAX_LOG_MSG_LEN MIN_TERM_COLS

typedef struct Logger {
    MenuChangeCounter data_id;
    uint8_t pending_msgs;
    size_t head;
    wchar_t msgs[MAX_LOG_MSGS][MAX_LOG_MSG_LEN];
} Logger;

Logger *init_logger(Logger *l);
void log_msg(Logger *l, wchar_t const *fmt, ...);
void _log_msg(Logger *l, char const *fmt, ...);
const wchar_t *nth_log_msg(Logger *l, unsigned int n);
bool log_has_pending(const Logger *l);

extern Logger g_game_log;
extern Logger g_debug_log;
