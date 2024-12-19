#include "log.h"

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdlib.h>

Logger g_game_log;
Logger g_debug_log;

Logger *init_logger(Logger *l)
{
    *l = (Logger) { 0, 0, 0, { { 0 } } };
    return l;
}

// TODO: This function truncates long messages...
void log_msg(Logger *l, wchar_t *fmt, ...)
{
    static wchar_t msgbuf[MAX_LOG_MSG_LEN];

    va_list va;
    va_start(va, fmt);
    vswprintf(msgbuf, MAX_LOG_MSG_LEN, fmt, va);
    msgbuf[MAX_LOG_MSG_LEN - 1] = L'\0';
    va_end(va);

    l->pending_msgs++, l->data_id++;
    wcsncpy(l->msgs[l->head], msgbuf, MAX_LOG_MSG_LEN);
    l->head = (l->head + 1) & (MAX_LOG_MSGS - 1);
}

void _log_msg(Logger *l, char *fmt, ...)
{
    static char msgbuf[MAX_LOG_MSG_LEN];

    va_list va;
    va_start(va, fmt);
    vsnprintf(msgbuf, MAX_LOG_MSG_LEN, fmt, va);
    msgbuf[MAX_LOG_MSG_LEN - 1] = L'\0';
    va_end(va);

    l->pending_msgs++, l->data_id++;
    mbstowcs(l->msgs[l->head], msgbuf, MAX_LOG_MSG_LEN);
    l->head = (l->head + 1) & (MAX_LOG_MSGS - 1);
}

const wchar_t *get_last_log_msg(Logger *l)
{
    if (l->pending_msgs > 0) l->pending_msgs--;
    return l->msgs[(l->head - 1) & (MAX_LOG_MSGS - 1)];
}

bool log_has_pending(const Logger *l)
{
    return l->pending_msgs;
}

// TODO: Make this less cancerous.
/// Caller owns allocated memory
int log_to_strs(Logger *l, wchar_t ***dest)
{
    int n_msgs = l->data_id > MAX_LOG_MSGS ? MAX_LOG_MSGS : l->data_id;
    if (n_msgs == 0) return 0;

    *dest = malloc(n_msgs * sizeof(**dest));
    if (!*dest) {
        perror("malloc");
        exit(1);
    }

    for (int i = 1; i <= n_msgs; i++)
        (*dest)[n_msgs - i] =  l->msgs[(l->head - i) & (MAX_LOG_MSGS - 1)];

    return n_msgs;
}
