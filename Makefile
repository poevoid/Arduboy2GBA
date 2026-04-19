DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM

CXX := $(DEVKITARM)/bin/arm-none-eabi-g++
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy

LIBGBA_INC := $(DEVKITPRO)/libgba/include
LIBGBA_LIB := $(DEVKITPRO)/libgba/lib

BUILD_VARS := build/build_vars.mk
FIXER := gbafix

ifeq ($(wildcard $(BUILD_VARS)),)
$(error Missing $(BUILD_VARS). Run the transpiler first.)
endif

include $(BUILD_VARS)

RUNTIME_SOURCES := \
	runtime/arduboy_compat.c \
	runtime/graphics.c \
	runtime/background.c \
	runtime/input.c \
	runtime/audio.c \
	runtime/gba_main.c

COMMON_FLAGS := \
	-mthumb \
	-mthumb-interwork \
	-fno-exceptions \
	-fno-rtti \
	-I$(LIBGBA_INC) \
	-I./runtime \
	-DTIME_SCALE=$(TIME_SCALE)

OPT_FLAGS := -O2

# Mystic Balloon specific build profile:
# Use stronger optimization and link-time optimization only for this game.
ifeq ($(GAME_PROFILE),MYBL_AB)
OPT_FLAGS := \
	-O3 \
	-flto \
	-fno-fat-lto-objects \
	-finline-functions \
	-finline-small-functions \
	-ffunction-sections \
	-fdata-sections
LINK_EXTRA_FLAGS := \
	-flto \
	-Wl,--gc-sections
else
LINK_EXTRA_FLAGS :=
endif

CXXFLAGS := -x c++ -std=gnu++17 $(OPT_FLAGS) $(COMMON_FLAGS)
LDFLAGS := -specs=gba.specs -L$(LIBGBA_LIB) $(LINK_EXTRA_FLAGS)
LDLIBS := -lgba -lm

.PHONY: all clean

all: $(OUTPUT_GBA)

$(OUTPUT_ELF): $(SOURCE_C) $(RUNTIME_SOURCES) runtime/arduboy_compat.h runtime/graphics.h runtime/input.h runtime/audio.h runtime/background.h
	$(CXX) $(CXXFLAGS) $(SOURCE_C) $(RUNTIME_SOURCES) -o $@ $(LDFLAGS) $(LDLIBS)

$(OUTPUT_GBA): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $< $@
	@$(FIXER) $@ >/dev/null 2>&1 || true
	@echo "ROM fixed!"

clean:
	rm -f $(OUTPUT_ELF) $(OUTPUT_GBA)
