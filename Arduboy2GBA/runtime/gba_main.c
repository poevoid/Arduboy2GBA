#include "arduboy_compat.h"
#include "audio.h"

extern void setup();
extern void loop();

int main() {
    ab_init();
    setup();

    while(1) {
        audio_update();
        loop();
    }
}
