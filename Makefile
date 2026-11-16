#---------------------------------------------------------------------------------
# Musictt - a native music player for Nintendo Switch
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET      := Musictt
BUILD       := build
SOURCES     := source
INCLUDES    := source vendor vendor/cjson vendor/dr vendor/stb

APP_TITLE   := Musictt
APP_AUTHOR  := ClmentCch
APP_VERSION := 1.0.0
APP_ICON    := $(CURDIR)/assets/icon.jpg

ARCH        := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC

CFLAGS      := -g -Wall -Wextra -Wno-unused-parameter -O2 -ffunction-sections -fdata-sections \
               $(ARCH) -D__SWITCH__ -DMUSICTT_VERSION=\"$(APP_VERSION)\"
CFLAGS      += $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2 -I$(LIBNX)/include

LDFLAGS     := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(BUILD)/$(TARGET).map -Wl,--gc-sections
LIBDIRS     := $(PORTLIBS) $(LIBNX)

# SDL2 and its codec/font dependencies come from the devkitPro portlibs; let
# pkg-config resolve the transitive static link flags instead of guessing names.
OBJCOPY     ?= $(PREFIX)objcopy

# devkitPro ships .pc files for the portlibs; make sure pkg-config finds them
export PKG_CONFIG_PATH := $(PORTLIBS)/lib/pkgconfig:$(PORTLIBS)/share/pkgconfig:$(PKG_CONFIG_PATH)
PKG_CONFIG  ?= pkg-config
SDL_PKGS    := sdl2 SDL2_image SDL2_ttf
SDL_CFLAGS  := $(shell $(PKG_CONFIG) --cflags $(SDL_PKGS) 2>/dev/null)
SDL_LIBS    := $(shell $(PKG_CONFIG) --libs --static $(SDL_PKGS) 2>/dev/null)

ifeq ($(strip $(SDL_LIBS)),)
$(warning pkg-config could not resolve SDL2; check dkp-pacman installs)
endif

CFLAGS      += $(SDL_CFLAGS)
LIBS        := -Wl,--start-group $(SDL_LIBS) -lnx -lm -Wl,--end-group

CFILES      := $(wildcard $(SOURCES)/*.c) $(wildcard vendor/cjson/*.c) $(wildcard vendor/stb/*.c)
OFILES      := $(patsubst %.c,$(BUILD)/%.o,$(CFILES)) $(BUILD)/font.o $(BUILD)/latin.o

.PHONY: all clean host-test

all: $(TARGET).nro

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(@:.o=.d) $(CFLAGS) -c $< -o $@

# Fonts are linked straight into .rodata: no romfs, no runtime file.
$(BUILD)/font.o: assets/MusicttCJK.ttc
	@mkdir -p $(dir $@)
	@echo embedding $(notdir $<)
	@cd $(CURDIR) && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		assets/MusicttCJK.ttc $@

$(BUILD)/latin.o: assets/NotoSans-Regular.ttf
	@mkdir -p $(dir $@)
	@echo embedding $(notdir $<)
	@cd $(CURDIR) && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		assets/NotoSans-Regular.ttf $@

LIBPATHS    := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

$(TARGET).elf: $(OFILES)
	@echo linking $(notdir $@)
	@$(CC) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
	@$(NM) -CSn $@ > $(TARGET).lst

$(TARGET).nacp: Makefile
	@nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@

$(TARGET).nro: $(TARGET).elf $(TARGET).nacp
	@elf2nro $< $@ --icon=$(APP_ICON) --nacp=$(TARGET).nacp
	@echo built ... $(notdir $@)

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro $(TARGET).nacp $(TARGET).lst

host-test:
	@bash scripts/host_test.sh

-include $(OFILES:.o=.d)
