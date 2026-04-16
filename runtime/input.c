#include <gba_input.h>
#include "input.h"

#define AB_KEY_MASK 0x03FF

static u16 curr = 0;
static u16 prev = 0;

static inline u16 read_keys_now(void) {
    return (u16)(~REG_KEYINPUT) & AB_KEY_MASK;
}

void input_poll(void) {
    prev = curr;
    curr = read_keys_now();
}

bool input_pressed(u16 key) {
    u16 now;

    if (key == 0) {
        return false;
    }

    now = read_keys_now();
    return (now & key) == key;
}

bool input_just_pressed(u16 key) {
    if (key == 0) {
        return false;
    }

    return ((curr & key) == key) && ((prev & key) != key);
}
