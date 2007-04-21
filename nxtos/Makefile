TARGET = nxtos
BUILD_DIR = build

C_SOURCES = aic.c systick.c twi.c sound.c avr.c main.c inutil.c
S_SOURCES = init.S interrupts.S vectors.S

OPT_FLAGS = -O3
#OPT_FLAGS = -O0 -g

##
## Do not edit past here.
##

include environment.mak

ROM_LDS      = $(BUILD_DIR)/$(TARGET)_rom.lds
SAMBA_LDS    = $(BUILD_DIR)/$(TARGET)_samba.lds
SOURCE_LDS   = $(TARGET).lds

S_OBJECTS    = $(addprefix $(BUILD_DIR)/, $(S_SOURCES:.S=.o))
C_OBJECTS    = $(addprefix $(BUILD_DIR)/, $(C_SOURCES:.c=.o) $(C_RAMSOURCES:.c=.oram))
ALL_OBJECTS  = $(C_OBJECTS) $(S_OBJECTS)

ROM_TARGET   = $(TARGET)_rom.bin
SAMBA_TARGET = $(TARGET)_samba.bin

ALL_TARGETS = $(SAMBA_TARGET) $(ROM_TARGET)

CFLAGS = -c -ffreestanding -fsigned-char -mcpu=arm7tdmi $(OPT_FLAGS) \
         -Wall -Werror -I. -ffunction-sections -fdata-sections -msoft-float \
         -mthumb-interwork

ASFLAGS = -Wall -Werror $(OPT_FLAGS) -Wa,-mcpu=arm7tdmi,-mfpu=softfpa,-mthumb-interwork

LIBS = $(LIBGCC)
LDFLAGS = -O3 -Map $@.map -cref --gc-sections

#
# Phony helper targets.
#
.PHONY: all rom samba clean ensure-build-dir
all: $(ALL_TARGETS)
rom: $(ROM_TARGET)
samba: $(SAMBA_TARGET)

clean:
	@echo "Removing all intermediary build output"
	@rm -rf $(BUILD_DIR)
	@echo "Removing targets"
	@rm -f $(ALL_TARGETS)
	@echo "Removing save files"
	@rm -f *~

ensure-build-dir:
	@mkdir -p $(BUILD_DIR)

#
# Final binary image targets.
#
$(SAMBA_TARGET): $(BUILD_DIR)/$(SAMBA_TARGET)_elf
	@echo "Creating binary image $@"
	$(OBJCOPY) -O binary $< $@

$(ROM_TARGET): $(BUILD_DIR)/$(ROM_TARGET)_elf
	@echo "Creating binary image $@"
	$(OBJCOPY) -O binary $< $@

#
# ELF images for each kind of build.
#
$(BUILD_DIR)/$(SAMBA_TARGET)_elf: ensure-build-dir $(ALL_OBJECTS) $(SAMBA_LDS)
	@echo "Linking ELF image $@"
	@cd $(BUILD_DIR)
	$(LD) -o $@ -T $(SAMBA_LDS) $(LDFLAGS) $(ALL_OBJECTS) $(LIBS)

$(BUILD_DIR)/$(ROM_TARGET)_elf: ensure-build-dir $(ALL_OBJECTS) $(ROM_LDS)
	@echo "Linking ELF image $@"
	$(LD) -o $@ $(ALL_OBJECTS) $(LIBS) -T $(ROM_LDS) $(LDFLAGS)

#
# Linker scripts for each kind of build.
#
$(SAMBA_LDS): $(SOURCE_LDS)
	@echo "Generating $@ from $<"
	cat $< | sed -e 's/SAMBA_ONLY//' -e '/RAM_ONLY/d' -e'/ROM_ONLY/d' >$@

$(ROM_LDS): $(SOURCE_LDS)
	@echo "Generating $@ from $<"
	cat $< | sed -e 's/ROM_ONLY//' -e'/RAM_ONLY/d' -e'/SAMBA_ONLY/d' >$@

#
# Automatic rules.
#
$(BUILD_DIR)/%.o: %.S
	@echo "Assembling $< to $@"
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.oram: %.c
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -o $@ $<
