#ifndef __NXTOS_MEMMAP_H__
#define __NXTOS_MEMMAP_H__

#include "mytypes.h"

/* The following constants are defined by GNU ld at the linking
 * phase. They describe the memory map of the NXT in terms of
 * symbols.
 */
extern U8 __samba_ram_start__;
extern U8 __samba_ram_end__;

extern U8 __ramtext_ram_start__;
extern U8 __ramtext_ram_end__;

extern U8 __text_start__;
extern U8 __text_end__;

extern U8 __data_ram_start__;
extern U8 __data_ram_end__;

extern U8 __bss_start__;
extern U8 __bss_end__;

extern U8 __stack_start__;
extern U8 __stack_end__;

extern U8 __free_ram_start__;
extern U8 __free_ram_end__;

/* Helper macro that converts a symbol value into a regular
 * integer. If we just addressed eg. __free_ram_start__ directly, the
 * C compiler would dereference and give us some random value (or a
 * data abort). If we just used eg. &__free_ram_start__, pointer
 * arithmetic would still apply.
 *
 * So, we need to grab the symbol's address, and then cast it to
 * U32. As the code to do this is a little ugly, it is defined once
 * here.
 */
#define SYMVAL(sym) ((U32)&(sym))

/* Wrapping of the raw symbols into something usable by the rest of
 * the kernel. We also define a _SIZE constant for each section, which
 * is just the number of bytes it uses.
 */
#define SAMBA_START SYMVAL(__samba_ram_start__)
#define SAMBA_END SYMVAL(__samba_ram_end__)
#define SAMBA_SIZE (SAMBA_END - SAMBA_START)

#define RAMTEXT_START SYMVAL(__ramtext_ram_start__)
#define RAMTEXT_END SYMVAL(__ramtext_ram_end__)
#define RAMTEXT_SIZE (RAMTEXT_END - RAMTEXT_START)

#define TEXT_START SYMVAL(__text_start__)
#define TEXT_END SYMVAL(__text_end__)
#define TEXT_SIZE (TEXT_END - TEXT_START)

#define DATA_START SYMVAL(__data_ram_start__)
#define DATA_END SYMVAL(__data_ram_end__)
#define DATA_SIZE (DATA_END - DATA_START)

#define BSS_START SYMVAL(__bss_start__)
#define BSS_END SYMVAL(__bss_end__)
#define BSS_SIZE (BSS_END - BSS_START)

#define STACK_START SYMVAL(__stack_start__)
#define STACK_END SYMVAL(__stack_end__)
#define STACK_SIZE (STACK_END - STACK_START)

#define FREE_START SYMVAL(__free_ram_start__)
#define FREE_END SYMVAL(__free_ram_end__)
#define FREE_SIZE (FREE_END - FREE_START)

/* If the kernel was booted from SAM-BA, the memory map defines an 8k
 * area reserved by SAM-BA. The ROM boot memory map sets this section
 * to zero size. Therefore, we can use the size of that section to
 * define two boolean constants, that tell the kernel how it was
 * booted.
 */
#define BOOTED_FROM_SAMBA (SAMBA_SIZE ? TRUE : FALSE)
#define BOOTED_FROM_ROM (!BOOTED_FROM_SAMBA)

#endif
