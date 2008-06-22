/** @file vm_opcodes.h
 *  @brief List of opcodes for the NXT.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPCODES_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPCODES_H__

enum {
  OP_ADD = 0x00,
  OP_SUB,
  OP_NEG,
  OP_MUL,
  OP_DIV,
  OP_MOD,

  OP_AND,
  OP_OR,
  OP_XOR,
  OP_NOT,

  /* Unused: 0x0A to 0x10 */

  OP_CMP = 0x11,
  OP_TST,

  /* Unused: 0x13, 0x14 */

  OP_INDEX = 0x15,
  OP_REPLACE,
  OP_ARRSIZE,
  OP_ARRBUILD,
  OP_ARRSUBSET,
  OP_ARRINIT,
  OP_MOV,
  OP_SET,
  OP_FLATTEN,
  OP_UNFLATTEN,
  OP_NUMTOSTRING,
  OP_STRINGTONUM,
  OP_STRCAT,
  OP_STRSUBSET,
  OP_STRTOBYTEARR,
  OP_BYTEARRTOSTR,

  OP_JMP,
  OP_BRCMP,
  OP_BRTST,

  OP_SYSCALL,

  OP_STOP,
  OP_FINCLUMP,
  OP_FINCLUMPIMMED,
  OP_ACQUIRE,
  OP_RELEASE,
  OP_SUBCALL,
  OP_SUBRET,

  OP_SETIN,
  OP_SETOUT,
  OP_GETIN,
  OP_GETOUT,

  /* Unused: 0x34 */

  OP_GETTICK = 0x35,

  NUM_OPCODES
} opcodes;

typedef void (*opcode_func)(void);

extern const opcode_func opcode_handlers[NUM_OPCODES];

void lego_vm_exec_opcode(void);

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPCODES_H__ */
