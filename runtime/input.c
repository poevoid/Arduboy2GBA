#include <gba_input.h>
#include "input.h"

#define AB_KEY_MASK 0x03FF

static u16 curr = 0;
static u16 prev = 0;

void input_poll(void) {
    prev = curr;
    curr = (u16)(~REG_KEYINPUT) & AB_KEY_MASK;
}

bool input_pressed(u16 key) {
    return (curr & key) != 0;
}

bool input_just_pressed(u16 key) {
    return ((curr & key) != 0) && ((prev & key) == 0);
}
