#include "input.h"

#include <gba_input.h>

static u16 g_prev = 0;
static u16 g_curr = 0;
static u16 g_just = 0;

void input_poll(void) {
    scanKeys();
    g_prev = g_curr;
    g_curr = keysHeld();
    g_just = (u16)(g_curr & (u16)~g_prev);
}

bool input_pressed(u16 key) {
    return (g_curr & key) == key;
}

bool input_just_pressed(u16 key) {
    return (g_just & key) == key;
}

u16 input_current(void) {
    return g_curr;
}

u16 input_previous(void) {
    return g_prev;
}

u16 input_just_pressed_mask(void) {
    return g_just;
}
