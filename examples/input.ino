#include <Arduboy2.h>

Arduboy2 arduboy;

void setup() {
  arduboy.begin();
}

void loop() {
  arduboy.clear();
  arduboy.drawRect(10, 10, 50, 20, 1);
  arduboy.setCursor(20, 40);
  arduboy.print("Hello GBA");
  arduboy.display();
}
