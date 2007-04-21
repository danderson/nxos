GCC_VERSION = 4.1.2
GCC_PATH = /mnt/data/arm-elf-gcc/build

# If you have a standard install of an arm-elf GCC cross-compiler, you
# should not have to edit any of the following. If you have a
# nonstandard installation, tweak below.

TARGET_PREFIX =arm-elf

INC_PATH  = $(GCC_PATH)/$(TARGET_PREFIX)/include
LIBGCC    = $(GCC_PATH)/lib/gcc/arm-elf/$(GCC_VERSION)/interwork/libgcc.a
LIBC      = $(GCC_PATH)/arm-elf/lib/interwork/libc.a

CC        = $(GCC_PATH)/bin/$(TARGET_PREFIX)-gcc
AS        = $(GCC_PATH)/bin/$(TARGET_PREFIX)-as
AR        = $(GCC_PATH)/bin/$(TARGET_PREFIX)-ar
LD        = $(GCC_PATH)/bin/$(TARGET_PREFIX)-ld
OBJCOPY   = $(GCC_PATH)/bin/$(TARGET_PREFIX)-objcopy
