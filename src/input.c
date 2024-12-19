#include "input.h"

#include "component.h"

#include "rlsmenu.h"
#include <uncursed/uncursed.h>

Position input_to_movement[] = {
    ['1'] = { -1, 1 }, ['2'] = { 0, 1 },   ['3'] = { 1, 1 },  ['4'] = { -1, 0 }, ['5'] = { 0, 0 },
    ['6'] = { 1, 0 },  ['7'] = { -1, -1 }, ['8'] = { 0, -1 }, ['9'] = { 1, -1 },
};

InputType get_input_type(KeyInfo *key)
{
    switch (key->status) {
    case OK:
        if (key->key != '5' && (key->key > 48 && key->key < 58))
            return MovementInput;
        else if (key->key == '.' || key->key == '5')
            return WaitInput;
        else if (key->key == ',')
            return PickupInput;
        else if (key->key == 'p')
            return PrayerInput;
    }

    return NotImplemented;
}

enum rlsmenu_input translate_key(KeyInfo *key)
{
    if (key->key >= 'a' && key->key <= 'z')
        return key->key - 'a';
    else if (key->key >= 'A' && key->key <= 'Z')
        return key->key - 'A';

    switch (key->status) {
    case OK:
        switch (key->key) {
        case '-':
            return RLSMENU_UP;
        case '+':
            return RLSMENU_DN;
        case '*':
            return RLSMENU_PGUP;
        case '/':
            return RLSMENU_PGDN;
        case 13: // Stupid libuncursed thing
            return RLSMENU_SEL;
        default:
            return RLSMENU_INVALID_KEY;
        }
    case KEY_CODE_YES:
        switch (key->key) {
        case KEY_ENTER:
            return RLSMENU_SEL;
        case KEY_ESCAPE:
            return RLSMENU_ESC;
        default:
            return RLSMENU_INVALID_KEY;
        }
    default:
        return RLSMENU_INVALID_KEY;
    }
}

#define TS_ENTER 0xa
#define TS_ESC   0x1b
#define TS_UP    0x41
#define TS_DN    0x42
#define TS_PGUP  0x35
#define TS_PGDN  0x36
int translate_sockui(char c, sockui_t *sui)
{
    if (c != 'q' && c >= 'a' && c <= 'z') return c - 'a';
    else if (c >= 'A' && c <= 'Z') return c - 'A';

    int ch;
    switch (c) {
        case TS_ENTER:
            return RLSMENU_SEL;
        case 'q':
            return RLSMENU_ESC;
        case TS_ESC:
            // Consume the [ if we're in an escape code. If we're not in an
            // escape code, this should time out (return 256). Note that on
            // older terminals it takes time to transmit each byte in the
            // escape sequence, and this doesn't wait at all. If you're using a
            // physical terminal, I guess you're SOL.
            if (sockui_recv(sui) == 256) return RLSMENU_ESC;
            // Return -1 here. This should bubble up elsewhere.
            if ((ch = sockui_recv(sui)) < 0 || ch > 255) return RLSMENU_INVALID_KEY;
            switch (ch) {
                case TS_UP:
                    return RLSMENU_UP;
                case TS_DN:
                    return RLSMENU_DN;
                case TS_PGUP:
                    return RLSMENU_PGUP;
                case TS_PGDN:
                    return RLSMENU_PGDN;
                default:
                    return RLSMENU_INVALID_KEY;
            }
        default:
            return RLSMENU_INVALID_KEY;
    }
}
#undef TS_ENTER
#undef TS_ESC
#undef TS_UP
#undef TS_DN
#undef TS_PGUP
#undef TS_PGDN
