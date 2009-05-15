/* Copyright (c) 2007,2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* NXT linker script source.
 *
 *  This file must be run through a sed script to generate an ldscript
 *  suitable for either a SAM-BA or rom image.
 *
 *  The sed script achieves this by selectively copying the lines with
 *  ROM_ONLY and SAM_ONLY.
 *
 *  Notes on alignment:
 *  1) Sections should be 4-byte aligned otherwise ARM fetches will be
 *     misaligned.
 *  2) The stack should be 8-byte aligned for the APCS. That's because
 *     STRD and LDRD assume that they are on 8-byte boundaries.
 *  3) Align 16 is a good idea for any copied or initialised sections
 *     because this allows the fatser LDM/STM code to run instead.
 */

/*
 * Memory definitions.
 */

MEMORY {
ROM_ONLY rom : ORIGIN = 1M, LENGTH =256k
vector_ram   : ORIGIN = 2M, LENGTH = 64
ram          : ORIGIN = 2M + 64, LENGTH = 64K - 64 + 21
}

ROM_ONLY ROM_BASE = 1M;
ROM_ONLY ROM_SIZE = 256k;
RAM_BASE = 2M;
RAM_SIZE = 64k;
SAMBA_RESERVED_SIZE = 8k;

SECTIONS {
  SAM_ONLY __boot_from_samba__ = 1;
  ROM_ONLY __boot_from_samba__ = 0;

  /*
   * Interrupt vectors. These are loaded to the bottom of memory at
   * boot time.
   */
  .vectors : {
    KEEP(base/vectors.o (*.text *.text.*))
    . = ALIGN(16);
  } > vector_ram
  ROM_ONLY AT> rom

  __vectors_ram_start__  = ADDR(.vectors);
  __vectors_ram_end__    = (ADDR(.vectors) + SIZEOF(.vectors));
  __vectors_load_start__ = LOADADDR(.vectors);
  __vectors_load_end__   = (LOADADDR(.vectors) + SIZEOF(.vectors));


  /*
   * The base bootstrapping code for the NXT.
   */
  .init : {
    base/init.o (*.text *.text.*)
    . = ALIGN(16);
  }
  ROM_ONLY > rom
  SAM_ONLY > ram


  /*
   * This section contains code that is relocated to RAM before
   * execution.
   *
   * This section is used for code that has to reside in ram to
   * function correctly (like flash writing code)
   */
  .ram_text : {
    . = ALIGN(16);
    *.oram(*.text *.text.* *.glue*) /* TODO: align? */
    *.oram(*.rodata *.rodata.*)
    . = ALIGN(16);
  } > ram
  ROM_ONLY AT> rom

  __ramtext_ram_start__ = ADDR(.ram_text);
  __ramtext_ram_end__   = ADDR(.ram_text) + SIZEOF(.ram_text);
  __ramtext_load_start__ = LOADADDR(.ram_text);
  __ramtext_load_end__ = __ramtext_load_start__ + SIZEOF(.ram_text) ;


  /*
   * Read/Write initialized data. Runs from RAM, gets copied there
   * from ROM if booting from flash.
   */
  .data : {
    * (*.data *.data.*)
    . = ALIGN(16);
  } > ram
  ROM_ONLY AT> rom

  __data_ram_start__ = ADDR(.data);
  __data_ram_end__   = ADDR(.data) + SIZEOF(.data);
  __data_load_start__ = LOADADDR(.data);
  __data_load_end__ = __data_load_start__ + SIZEOF(.data) ;


  /*
   * The main kernel executable code, as well as all the read-only
   * data. If possible, this goes into flash, to free up more RAM.
   */
  .text : {
    . = ALIGN(16);
    * (*.text *.text.* *.glue*)
    . = ALIGN(16);
    * (*.rodata *.rodata.*)
    . = ALIGN(16);
  }
  ROM_ONLY > rom
  SAM_ONLY > ram

  ROM_ONLY __rom_userspace_start__ = ADDR(.text) + SIZEOF(.text);
  SAM_ONLY __rom_userspace_start__ = 0;

  __text_start__ = ADDR(.text);
  __text_end__ = ADDR(.text) + SIZEOF(.text);

  /*
   * The BSS section is zero-initialized data. The section does not
   * take any space in the final image, but the linker helpfully
   * defines the symbols we need to be able to initialize the section
   * properly.
   */
  .bss : {
    * (*.bss *.bss.*)
    . = ALIGN(16);
    __bss_end__ = . ;
  } > ram

  __bss_start__ = ADDR(.bss);
  __bss_end__   = (ADDR(.bss) + SIZEOF(.bss));


  /*
   * The various kernel stacks.
   *
   * Although not strictly required, this zone is zeroed out at boot
   * time to provide a clean environment for the kernel.
   */
  .stack  : {
    /* irq stack */
    . = ALIGN(16);
    . += 0x60; /* 96b, which is 3*8*4, the maximum irq stack size. */
    . = ALIGN(8);
    __irq_stack__ = .;

    /* supervisor stack */
    __supervisor_stack_bottom__ = . ;
    . += 0x400; /* 1k supervisor stack. */
    . = ALIGN(16);
    __supervisor_stack__ = .;
    __supervisor_stack_top__ = . ;
  } > ram

  __stack_start__ = ADDR(.stack);
  __stack_end__ = (ADDR(.stack) + SIZEOF(.stack));

  SAM_ONLY __kernel_ram_load_size__ = SIZEOF(.vectors) + SIZEOF(.init) +
  SAM_ONLY                            SIZEOF(.ram_text) + SIZEOF(.text) +
  SAM_ONLY                            SIZEOF(.data);

  /* SAMBA stage 0.5 */
  SAM_ONLY .samba_boot ADDR(.bss) : {
  SAM_ONLY   KEEP(base/samba_init.o(.samba_init));
  SAM_ONLY }

  /*
   * Variables defined to inform the kernel of how much RAM remains
   * behind the loaded kernel.
   */
  /* TODO: fix these values. */
  __userspace_start__ = __stack_end__;
  __userspace_end__   = (RAM_BASE + RAM_SIZE);

  __free_ram_start__ = __stack_end__ ;
  __free_ram_end__ = (RAM_BASE + RAM_SIZE);

  /*
   * The magic metadata struct is tacked onto the start of the kernel binary.
   */
  SAM_ONLY .magic ADDR(.samba_boot) + SIZEOF(.samba_boot) : {
  ROM_ONLY .magic : {
    /* Magic identifier */
    LONG(0xDEADBEEF);
    /* Kernel in-ram size */
    __kernel_in_ram_size__ =  SIZEOF(.vectors) + SIZEOF(.init) +
      SIZEOF(.ram_text) + SIZEOF(.data) +
      SIZEOF(.bss) + SIZEOF(.stack)
      SAM_ONLY + SIZEOF(.text)
      ;
    LONG(__kernel_in_ram_size__);
    /* Wasted ram space (after the kernel). TODO: remove */
    SAM_ONLY LONG(0);
    ROM_ONLY LONG(0);
    /* Kernel in-rom size */
    SAM_ONLY LONG(0);
    ROM_ONLY   LONG(SIZEOF(.vectors) + SIZEOF(.init) +
    ROM_ONLY        SIZEOF(.ram_text) + SIZEOF(.data) + SIZEOF(.text));
    /* Bootup address */
    SAM_ONLY LONG(ADDR(.samba_boot) + SAMBA_RESERVED_SIZE);
    ROM_ONLY LONG(0x100000);
    /* SAM-BA kernel? */
    SAM_ONLY BYTE(1);
    ROM_ONLY BYTE(0);
  }
  SAM_ONLY > ram
  ROM_ONLY > rom

  /* Discard anything not mapped at this point. */
  /DISCARD/ : {
    * (*);
  }
}
