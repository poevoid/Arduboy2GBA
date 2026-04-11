
#include "../runtime/arduboy_compat.h"


void setup() {
  
}

void loop() {
  ab_clear();
  ab_drawRect(10, 10, 50, 20, 1);
  ab_setCursor(20, 40);
  ab_print("Hello GBA");
  ab_display();
}
