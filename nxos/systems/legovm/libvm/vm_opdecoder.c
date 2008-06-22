/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"

#include "vm_opdecoder.h"
#include "vm_header.h"

typedef struct __attribute__ ((__packed__)) {
  /* Opcode in the long form, argument/offset in short form. */
  union {
    U8 opcode_or_arg;
    S8 offset;
  } opcode_or_arg_or_offset;

  /* Flags in long form, opcode in short form. */
  U32 flags_or_opcode :3;

  /* Should be 0 in long form, 1 in short form. */
  U32 is_short :1;

  /* Total opcode size in bytes. */
  U32 size :4;
} opcode;

const U16 *lego_vm_decode_instruction(const U16* pc) {
  opcode *op = (opcode*) pc;

  vm.instruction.num_operands = (op->size - 2) / 2;

  if (op->is_short) {
    vm.instruction.opcode = op->flags_or_opcode;

    if (vm.instruction.num_operands == 1) {
      vm.instruction.arg1 = op->opcode_or_arg_or_offset.opcode_or_arg;
    } else {
      vm.instruction.arg2 = pc[1];
      vm.instruction.arg1 =
        vm.instruction.arg2 + op->opcode_or_arg_or_offset.offset;
    }
  } else {
    vm.instruction.opcode = op->opcode_or_arg_or_offset.opcode_or_arg;
    vm.instruction.comparison_code = op->flags_or_opcode;

    if (vm.instruction.num_operands > 0)
      vm.instruction.arg1 = pc[1];
    if (vm.instruction.num_operands > 1)
      vm.instruction.arg2 = pc[2];
    vm.instruction.other_args = &pc[3];
  }

  return pc + vm.instruction.num_operands + 1;
}
