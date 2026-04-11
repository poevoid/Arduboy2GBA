# Arduboy2GBA

A small transpiler + runtime SDK that converts simple Arduboy/Arduino sketches into a native `.gba` ROM.

This project works by:

1. reading an Arduboy `.ino` sketch
2. mapping supported Arduboy APIs to a GBA compatibility layer
3. compiling the generated C source with `devkitARM` + `libgba`
4. producing a `.gba` ROM named after the input sketch

For example:

- `examples/input.ino` → `input.gba`
- `examples/thisgame.ino` → `thisgame.gba`

---

## Project layout
```
Arduboy2GBA/
├── transpiler/
│   ├── main.py
│   ├── parser.py
│   ├── mapper.py
│   ├── bitmap.py
│   └── audio.py
├── runtime/
│   ├── gba_main.c
│   ├── arduboy_compat.h
│   ├── arduboy_compat.c
│   ├── graphics.h
│   ├── graphics.c
│   ├── input.h
│   ├── input.c
│   ├── audio.h
│   └── audio.c
├── build/
├── assets/
├── examples/
├── Makefile
└── README.md
```
## Current capabilities

### Transpiler
- parses an input .ino
- removes Arduboy/Arduino-specific includes and object declarations
- maps a subset of Arduboy, Arduboy2, Playtune, and Sprites APIs to GBA runtime functions
- generates build/<inputname>.c
- generates build/build_vars.mk so the Makefile knows what to build

### Runtime
- centered 128x64 rendering inside the GBA 240x160 screen
- pixel, line, rectangle, and fill drawing
- Arduboy-style 6x8 text cells using a 5x7 font
- proper Arduboy bit-packed bitmap decoding
- sprite modes supported:
  - overwrite
  - self masked
  - erase
  - plus mask
  - external mask
- button input polling
- basic tone and Playtune-style audio

---

## Requirements

- Windows
- devkitPro toolchain
- devkitARM
- libgba (gba-dev package)
- MSYS2 or devkitPro shell
- Python 3 installed in the same shell you will run the transpiler

---

## Installing devkitPro and dependencies

1. Install devkitPro using the official installer
2. Open the devkitPro MSYS2 shell

Update system

pacman -Syu

If prompted, close and reopen shell, then run

pacman -Su

Install GBA toolchain

pacman -S gba-dev

---

## Installing Python in MSYS2

Inside the devkitPro MSYS2 shell

pacman -S python

Verify installation

python --version

Optional pip setup

python -m ensurepip
python -m pip install --upgrade pip

---

## Verify toolchain

python --version
make --version
arm-none-eabi-gcc --version

---

## Building a ROM

Place your sketch

examples/hello.ino

From transpiler directory

python main.py -o ../examples/hello.ino

Then from project root

make

Output

hello.elf
hello.gba

---

## One command build

From transpiler directory

python main.py -o ../examples/hello.ino --run

This will:
- transpile the sketch
- generate build files
- invoke make
- produce hello.gba

---

## Command line options

Input file

python main.py -o ../examples/game.ino

Aliases supported
-i
-o
--input

Run build automatically

python main.py -o ../examples/game.ino --run

Custom make command

python main.py -o ../examples/game.ino --run --make-cmd make

---

## Output naming

Input

../examples/mygame.ino

Outputs

build/mygame.c
mygame.elf
mygame.gba

---

## Example sketch

#include <Arduboy2.h>

Arduboy2 arduboy

void setup() {
  arduboy.begin()
}

void loop() {
  arduboy.clear()
  arduboy.drawRect(10, 10, 50, 20, WHITE)
  arduboy.setCursor(20, 40)
  arduboy.print("Hello GBA")
  arduboy.display()
}

Run

python main.py -o ../examples/input.ino --run

---

## Supported APIs

Drawing
drawPixel
drawFastHLine
drawFastVLine
drawRect
fillRect
fillScreen
drawBitmap

Text
setCursor
print string

Sprites
Sprites drawOverwrite
Sprites drawSelfMasked
Sprites drawErase
Sprites drawPlusMask
Sprites drawExternalMask

Input
pollButtons
pressed

Audio
tone
playScore
stopScore

---

## Limitations

- not all Arduboy APIs are implemented
- print only supports strings cleanly
- limited Arduino C++ support
- timing differences vs real hardware
- audio is basic
- some games require manual fixes

---

## Troubleshooting

Missing Arduboy2.h
re run transpiler and rebuild

Missing gba headers
ensure gba-dev installed and correct shell used

Python not found
install with pacman in MSYS2 shell

Black screen
ensure display is called in loop

Rendering issues
may require additional API support

---

## Workflow summary

Option 1

python main.py -o ../examples/game.ino
make

Option 2

python main.py -o ../examples/game.ino --run
