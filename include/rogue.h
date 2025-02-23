#pragma once

#define INVENTORY_MAX 64
#define NAME_LEN_MAX 64

#define MIN_TERM_LINES 24
#define MIN_TERM_COLS 80

#define alpha_to_idx(c) ((c) < 97 ? (c) -39 : (c) -97)
#define is_alpha(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
// TODO: Unify macro convention to be caps
#define XY_TO_IDX(x, y, w) ((x) + (y) * (w))

extern int X_DIRS[8];
extern int Y_DIRS[8];

typedef unsigned int MenuChangeCounter;
typedef unsigned long int ecs_entity_t;
extern ecs_entity_t g_player_id;

typedef enum { RunSystems, PreTurn, PlayerTurn, GUI, NewGUIFrame } GameState;
typedef enum {
    PlayerGUICommand,
    GUICommand,
    HeroCommand,
    QuitCommand,
    InvalidCommand,
    CancelCommand,
    SuccessCommand
} CommandType;

typedef struct WINDOW WINDOW;
typedef struct rlsmenu_gui rlsmenu_gui;

typedef struct {
    GameState state;
    WINDOW *basewin;
    WINDOW *logwin;
    WINDOW *statuswin;
    rlsmenu_gui *gui;
} GameVars;

