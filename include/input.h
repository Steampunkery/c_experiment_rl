#pragma once
#include "component.h"

typedef unsigned int wint_t;
typedef enum { MovementInput, WaitInput, PickupInput, PrayerInput, NotImplemented } InputType;

extern Position input_to_movement[];

typedef struct KeyInfo {
    int status;
    wint_t key;
} KeyInfo;

InputType get_input_type(KeyInfo *key);
enum rlsmenu_input translate_key(KeyInfo *key);
int translate_sockui(char c, sockui_t *sui);
