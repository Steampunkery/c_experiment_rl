#pragma once
#include "component.h"

typedef unsigned int wint_t;
typedef enum { MovementInput, WaitInput, PickupInput, PrayerInput, NotImplemented } InputType;
typedef struct Position MovementAction;

extern MovementAction input_to_movement[];

typedef struct KeyInfo {
    int status;
    wint_t key;
} KeyInfo;

InputType get_input_type(KeyInfo *key);
