#include "input.h"
#include "component.h"
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
