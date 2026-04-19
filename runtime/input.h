#ifndef INPUT_H
#define INPUT_H

#include <gba_types.h>
#include <stdbool.h>

void input_poll(void);
bool input_pressed(u16 key);
bool input_just_pressed(u16 key);
u16 input_current(void);
u16 input_previous(void);
u16 input_just_pressed_mask(void);

#endif
