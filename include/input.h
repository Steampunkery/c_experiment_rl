#pragma once
#include "component.h"

typedef unsigned int wint_t;
typedef enum { MovementInput, WaitInput, PickupInput, PrayerInput, NotImplemented } InputType;

extern Position direction8[];
extern Position direction9[];

typedef struct KeyInfo {
    int status;
    wint_t key;
} KeyInfo;

CommandType get_command(KeyInfo *key, int msec_timeout);
InputType get_input_type(KeyInfo *key);
enum rlsmenu_input translate_key(KeyInfo *key);
int translate_sockui(int c, sockui_t *sui);
