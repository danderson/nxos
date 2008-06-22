/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include <stdio.h>

#include "base/types.h"
#include "vm.h"

#define BUF_SIZE (40*1024)
U8 program[BUF_SIZE];

static bool load_program(const char* filename) {
  FILE *f = fopen(filename, "rb");
  if (f == NULL)
    return FALSE;
  fread(program, BUF_SIZE, 1, f);
  fclose(f);
  return TRUE;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Syntax: %s <RXE file>\n", argv[0]);
    return 1;
  }
  if (load_program(argv[1]) &&
      lego_vm_init(program)) {
    lego_vm_run();
    lego_vm_destroy();
    return 0;
  } else {
    return 1;
  }
}
