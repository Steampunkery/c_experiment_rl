#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define INVENTORY_MAX 64
#define NAME_LEN_MAX 64

#define MIN_TERM_LINES 24
#define MIN_TERM_COLS 80

extern char *idx_to_alpha;
#define alpha_to_idx(c) ((c) < 97 ? (c) - 39 : (c) - 97)
#define is_alpha(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

typedef uint64_t ecs_entity_t;
extern ecs_entity_t g_player_id;

typedef enum { RunSystems, TakeInput, GUI, NewGUIFrame } GameState;
typedef enum { PlayerGUICommand, GUICommand, HeroCommand, QuitCommand, InvalidCommand, CancelCommand, SuccessCommand } CommandType;

static inline bool assert_bool(bool cond) { assert(cond); return true; }
