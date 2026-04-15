#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include "arduboy_compat.h"

#ifndef TIME_SCALE
#define TIME_SCALE 1.0f
#endif

extern void setup(void);
extern void loop(void);

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);

    ab_begin();
    ab_setTimeScale((float)TIME_SCALE);

    setup();

    while (1) {
        loop();
    }
}
