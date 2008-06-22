/** @file vm_opdecoder.h
 *  @brief Instruction decoder for the Lego VM.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPDECODER_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPDECODER_H__

#include "base/types.h"

/* Decodes one instruction at the given PC into the VM instruction
 * scratch buffer, and returns the new PC pointing past that
 * instruction.
 */
const U16 *lego_vm_decode_instruction(const U16 *pc);

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_OPDECODER_H__ */
