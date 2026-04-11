#include "input.h"
#include <gba_input.h>

static u16 keys;

/* GBA has 10 main button bits in REG_KEYINPUT */
#define AB_GBA_KEY_MASK 0x03FF

u16 input_poll() {
    keys = (~REG_KEYINPUT) & AB_GBA_KEY_MASK;
    return keys;
}

int input_pressed(u16 key) {
    return (keys & key) != 0;
}
