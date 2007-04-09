/* Definition of the memory regions in the NXT */

MEMORY {
  rom : ORIGIN = 1M, LENGTH = 256k
  vector_ram : ORIGIN = 2M, LENGTH = 64
  ram : ORIGIN = 2M + 64, LENGTH = 64k - 64
}

RAM_START = 2M;
RAM_SIZE = 64k;


SECTIONS {
  /* If the kernel is loaded by SAM-BA, reserve the bottom 8k of ram. */
  SAMBA_ONLY .samba_reserved : {
  SAMBA_ONLY   . = (8k - 64);
  SAMBA_ONLY } > ram


  /* Interrupt vectors. These are loaded to the bottom of memory at boot time. */
  .vectors : {
    KEEP(build/vectors.o (.vectors))
    . = ALIGN(16);
  } > vector_ram
  ROM_ONLY AT > rom
  SAMBA_ONLY AT > ram
  __vectors_ram_start = ADDR(.vectors);
  __vectors_load_start = LOADADDR(.vectors);
  __vectors_load_end = (__vectors_load_start + SIZEOF(.vectors));


  /* The fundamental bootstrapping code for the NXT. */
  .bootstrap : {
    build/crt0_s.o(*.text)
    build/crt0_c.o(*.text)
    . = ALIGN(16);
  }
  ROM_ONLY > rom
  SAMBA_ONLY > ram


  /* Ramtext is code that needs to be in RAM to execute properly. This
   * is for example the case of the flash driver, which cannot be in
   * flash while flash is being written to.
   */
  .ram_text : {
    . = ALIGN(16);
    *.oram(*.text *.glue*)
    . = ALIGN(16);
    *.oram(*.rodata)
    . = ALIGN(16);
  } > ram
  ROM_ONLY AT > rom
  __ramtext_ram_start = ADDR(.ram_text);
  __ramtext_load_start = LOADADDR(.ram_text);
  __ramtext_load_end = (__ramtext_load_start + SIZEOF(.ram_text));


  .data : {
    *.o(*.data)
    . = ALIGN(16);
  } > ram
  ROM_ONLY AT > rom
  __data_ram_start = ADDR(.data);
  __data_load_start = LOADADDR(.data);
  __data_load_end = (__data_load_start + SIZEOF(.data));


  /* The main code and other read-only data for the kernel. This can
   * stay in rom for a rom build. */
  .text : {
    . = ALIGN(16);
    *.o(*.text)
    *.o(*.rodata)
    *(.glue*) /* generated Arm-Thumb interworking code. */
    . = ALIGN(16);
  }
  ROM_ONLY > rom
  SAMBA_ONLY > ram


  /* The stacks are placed in ram. There is a small IRQ stack,
   * followed by a larger system stack.
   */
  .stack : {
    /* IRQ stack */
    . += 0x60;
    . = ALIGN(16);
    __irq_stack = .;

    /* System stack. The system gets 1k for the stack. */
    __system_stack_bottom = .;
    . += 0x400;
    . = ALIGN(16);
    __system_stack = .;
  } > ram
  __stack_start = ADDR(.stack);
  __stack_end = (__stack_start + SIZEOF(.stack));


  /* The BSS section contains zero-initialized data, and should be
   * placed in ram.
   */
  .bss : {
    *(*.bss)
    . = ALIGN(16);
  } > ram
  __bss_start = ADDR(.bss);
  __bss_end = (__bss_start + SIZEOF(.bss));

  __free_ram_start = __bss_end;
  __free_ram_end = (RAM_START + RAM_SIZE);

  FREE_RAM = (__free_ram_end - __free_ram_start);

  /DISCARD/ : {
  *(.comment)
  }
}
