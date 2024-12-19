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
void log_msg(Logger *l, wchar_t *fmt, ...);
void _log_msg(Logger *l, char *fmt, ...);
const wchar_t *get_last_log_msg(Logger *l);
bool log_has_pending(const Logger *l);
int log_to_strs(Logger *l, wchar_t ***dest);

extern Logger g_game_log;
extern Logger g_debug_log;
