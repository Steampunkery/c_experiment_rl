#include "input.h"
#include "component.h"
#include "rlsmenu.h"
#include <uncursed/uncursed.h>

MovementAction input_to_movement[] = {
    ['1'] = { -1, 1 },
    ['2'] = { 0, 1 },
    ['3'] = { 1, 1 },
    ['4'] = { -1, 0 },
    ['5'] = { 0, 0 },
    ['6'] = { 1, 0 },
    ['7'] = { -1, -1 },
    ['8'] = { 0, -1 },
    ['9'] = { 1, -1 },
};

InputType get_input_type(KeyInfo *key) {
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

enum rlsmenu_input translate_key(KeyInfo *key) {
    if (key->key >= 'a' && key->key <= 'z') return key->key - 'a';
    else if (key->key >= 'A' && key->key <= 'Z') return key->key - 'A';

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
