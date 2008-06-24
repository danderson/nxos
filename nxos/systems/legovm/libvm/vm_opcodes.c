/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/assert.h"

#include "vm_opcodes.h"
#include "vm_header.h"

#include "vm_op_arithmetic.h"
#include "vm_op_logic.h"
#include "vm_op_flowcontrol.h"

const opcode_func opcode_handlers[NUM_OPCODES] = {
  lego_vm_op_add, /* OP_ADD */
  lego_vm_op_sub, /* OP_SUB */
  lego_vm_op_neg, /* OP_NEG */
  lego_vm_op_mul, /* OP_MUL */
  lego_vm_op_div, /* OP_DIV */
  lego_vm_op_mod, /* OP_MOD */

  lego_vm_op_and, /* OP_AND */
  lego_vm_op_or,  /* OP_OR */
  lego_vm_op_xor, /* OP_XOR */
  lego_vm_op_not, /* OP_NOT */

  NULL, /* Unused */
  NULL, /* Unused */
  NULL, /* Unused */
  NULL, /* Unused */
  NULL, /* Unused */
  NULL, /* Unused */
  NULL, /* Unused */

  NULL, /* OP_CMP */
  NULL, /* OP_TST */

  NULL, /* Unused */
  NULL, /* Unused */

  NULL, /* OP_INDEX */
  NULL, /* OP_REPLACE */
  NULL, /* OP_ARRSIZE */
  NULL, /* OP_ARRBUILD */
  NULL, /* OP_ARRSUBSET */
  NULL, /* OP_ARRINIT */
  NULL, /* OP_MOV */
  NULL, /* OP_SET */
  NULL, /* OP_FLATTEN */
  NULL, /* OP_UNFLATTEN */
  NULL, /* OP_NUMTOSTRING */
  NULL, /* OP_STRINGTONUM */
  NULL, /* OP_STRCAT */
  NULL, /* OP_STRSUBSET */
  NULL, /* OP_STRTOBYTEARR */
  NULL, /* OP_BYTEARRTOSTR */

  NULL, /* OP_JMP */
  NULL, /* OP_BRCMP */
  NULL, /* OP_BRTST */

  NULL, /* OP_SYSCALL */

  NULL, /* OP_STOP */
  lego_vm_op_finclump, /* OP_FINCLUMP */
  lego_vm_op_finclumpimmed, /* OP_FINCLUMPIMMED */
  NULL, /* OP_ACQUIRE */
  NULL, /* OP_RELEASE */
  NULL, /* OP_SUBCALL */
  NULL, /* OP_SUBRET */

  NULL, /* OP_SETIN */
  NULL, /* OP_SETOUT */
  NULL, /* OP_GETIN */
  NULL, /* OP_GETOUT */

  NULL, /* Unused */

  NULL, /* OP_GETTICK */
};

void lego_vm_exec_opcode(void) {
  NX_ASSERT(vm.instruction.opcode < NUM_OPCODES);

  const opcode_func handler = opcode_handlers[vm.instruction.opcode];

  if (handler == NULL) {
    vm.state = CRASHED;
    return;
  }

  handler();
}
