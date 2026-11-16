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
OBJCOPY     ?= $(PREFIX)objcopy

# Link every archive devkitPro installed for the Switch portlibs. The static
# linker only pulls the objects it needs, and --start-group makes the ordering
# of SDL2/freetype/png/jpeg/webp/mesa irrelevant. pkg-config is not used: the
# pinned devkitPro image does not export .pc paths for cross builds.
PORTLIBS_ARCHIVES := $(wildcard $(PORTLIBS)/lib/*.a)

CFLAGS      := -g -Wall -Wextra -Wno-unused-parameter -O2 -ffunction-sections -fdata-sections \
               $(ARCH) -D__SWITCH__ -DMUSICTT_VERSION='"$(APP_VERSION)"'
CFLAGS      += $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
               -I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2 -I$(LIBNX)/include

LDFLAGS     := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
               -Wl,-Map,$(BUILD)/$(TARGET).map -Wl,--gc-sections
LIBPATHS    := -L$(PORTLIBS)/lib -L$(LIBNX)/lib
# Mesa's EGL/glapi archives are C++; the C driver still needs the C++ runtime.
LIBS        := -Wl,--start-group $(PORTLIBS_ARCHIVES) -lnx -lstdc++ -lm -Wl,--end-group

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
