include environment.mak

TARGET = estorm
BUILD = build

S_SOURCES = crt0_s.S vectors.S
C_SOURCES = crt0_c.c main.c aic.c sys_timer.c twi.c

ROM_TARGET = $(TARGET)_rom.bin
SAMBA_TARGET  = $(TARGET)_samba.bin

SOURCE_LDSCRIPT = $(TARGET).lds
ROM_LDSCRIPT = $(BUILD)/$(TARGET)_rom.lds
SAMBA_LDSCRIPT = $(BUILD)/$(TARGET)_samba.lds

S_OBJECTS = $(addprefix $(BUILD)/,$(S_SOURCES:.S=.o))
C_OBJECTS = $(addprefix $(BUILD)/,$(C_SOURCES:.c=.o))

OPTIMIZE=-O2

CCFLAGS=-ffreestanding -W -Wall -Werror $(OPTIMIZE) -mcpu=arm7tdmi -msoft-float -mfpu=fpa -mapcs -mthumb-interwork -Isupport
ASFLAGS=-W -Wall -Werror $(OPTIMIZE) -Wa,-mcpu=arm7tdmi,-mapcs-32,-mthumb-interwork,-mfpu=softfpa
LDFLAGS=-N -Map $@.map -nostdlib $(OPTIMIZE) -cref --gc-sections -L$(BUILD)

all: $(ROM_TARGET) $(SAMBA_TARGET)

$(SAMBA_LDSCRIPT): $(SOURCE_LDSCRIPT)
	cat $< | sed -E -e 's/^ *SAMBA_ONLY//' -e '/^ *ROM_ONLY/d' >$@

$(ROM_LDSCRIPT): $(SOURCE_LDSCRIPT)
	cat $< | sed -E -e 's/^ *ROM_ONLY//' -e '/^ *SAMBA_ONLY/d' >$@

$(SAMBA_TARGET): $(BUILD)/$(SAMBA_TARGET)_elf
	$(OBJCOPY) -O binary $< $@

$(ROM_TARGET): $(BUILD)/$(ROM_TARGET)_elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(SAMBA_TARGET)_elf: $(C_OBJECTS) $(S_OBJECTS) $(SAMBA_LDSCRIPT)
	$(LD) -o $@ $(C_OBJECTS) $(S_OBJECTS) -T $(SAMBA_LDSCRIPT) $(LDFLAGS)

$(BUILD)/$(ROM_TARGET)_elf: $(C_OBJECTS) $(S_OBJECTS) $(ROM_LDSCRIPT)
	$(LD) -o $@ $(C_OBJECTS) $(S_OBJECTS) -T $(ROM_LDSCRIPT) $(LDFLAGS)

$(BUILD)/%.o: %.c
	$(GCC) $(CCFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.S
	$(GCC) $(ASFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD) $(SAMBA_TARGET) $(ROM_TARGET)
	mkdir $(BUILD)
